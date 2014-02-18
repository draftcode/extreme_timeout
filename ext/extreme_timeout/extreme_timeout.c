#include <ruby.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#if defined(HAVE_BACKTRACE) && !defined(__APPLE__)
# include <execinfo.h>
#endif

static pthread_mutex_t exitcode_mutex = PTHREAD_MUTEX_INITIALIZER;
static int exitcode;

struct wait_args {
    unsigned int timeout_sec;
    int exitcode;
    pthread_t running_thread;
};

static void
stacktrace_dumper(int signum)
{
    #define MAX_TRACES 1024
    static void *trace[MAX_TRACES];
    int n;
    VALUE backtrace_arr;

    backtrace_arr = rb_make_backtrace();
    fprintf(stderr,
            "-- Ruby level backtrace --------------------------------------\n");
    rb_io_puts(1, &backtrace_arr, rb_stderr);
    fprintf(stderr, "\n");

    fprintf(stderr,
            "-- C level backtrace -----------------------------------------\n");
#if defined(HAVE_BACKTRACE) && !defined(__APPLE__)
    n = backtrace(trace, MAX_TRACES);
    backtrace_symbols_fd(trace, n, STDERR_FILENO);
#elif defined(HAVE_BACKTRACE) && defined(__APPLE__)
    fprintf(stderr,
            "C level backtrace is unavailable due to the bug in the OSX's environment.\nSee r39301 and r39808 of CRuby for the details.\n");
#else
    fprintf(stderr,
            "C level backtrace is unavailable because backtrace(3) is unavailable.\n")
#endif
    exit(exitcode);
}

static void
set_stacktrace_dumper(void)
{
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_handler = stacktrace_dumper;
    sigaction(SIGCONT, &sa, NULL);
}

void *
sleep_thread_main(void *_arg)
{
    struct wait_args *arg = _arg;
    sleep(arg->timeout_sec);
    fprintf(stderr, "Process exits(ExtremeTimeout::timeout)\n");
    fflush(stderr);

    pthread_mutex_lock(&exitcode_mutex);
    exitcode = arg->exitcode;
    if (!rb_during_gc()) {
      set_stacktrace_dumper();
      if (pthread_kill(arg->running_thread, SIGCONT) == 0) {
          pthread_join(arg->running_thread, NULL);
      }
    } else {
      exit(exitcode);
    }
    return NULL;
}

static VALUE
timeout_cb(VALUE block)
{
    return rb_funcall(block, rb_intern("call"), 0);
}

VALUE
timeout(int argc, VALUE *argv, VALUE self)
{
    int exitcode = 1, state;
    unsigned int timeout_sec = 0;
    VALUE timeout_sec_value, exitcode_value, block;
    pthread_t thread;
    struct wait_args arg;
    VALUE retval;

    rb_scan_args(argc, argv, "11&", &timeout_sec_value, &exitcode_value, &block);

    if (!FIXNUM_P(timeout_sec_value)) {
        rb_raise(rb_eArgError, "the timeout argument should be Fixnum");
    }
    timeout_sec = FIX2UINT(timeout_sec_value);

    exitcode = 1;
    if (exitcode_value != Qnil) {
        if (!FIXNUM_P(exitcode_value)) {
            rb_raise(rb_eArgError, "the exitcode argument should be Fixnum");
        }
        exitcode = FIX2INT(exitcode_value);
    }

    if (block == Qnil) {
        rb_raise(rb_eArgError, "expects block");
    }

    arg.timeout_sec = timeout_sec;
    arg.exitcode = exitcode;
    arg.running_thread = pthread_self();
    if (pthread_create(&thread, NULL, sleep_thread_main, &arg) != 0) {
        rb_raise(rb_eRuntimeError, "pthread_create was failed");
    }

    retval = rb_protect(timeout_cb, block, &state);

    pthread_cancel(thread);
    pthread_join(thread, NULL);
    return retval;
}

void
Init_extreme_timeout(void)
{
    VALUE module = rb_define_module("ExtremeTimeout");
    rb_define_module_function(module, "timeout", timeout, -1);
}
