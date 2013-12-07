#include <ruby.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct wait_args {
    unsigned int timeout_sec;
    int exitcode;
};

void *
sleep_thread_main(void *_arg)
{
    struct wait_args *arg = _arg;
    sleep(arg->timeout_sec);
    fprintf(stderr, "Process exits(ExtremeTimeout::timeout)\n");
    fflush(stderr);
    exit(arg->exitcode);
}

VALUE
timeout(int argc, VALUE *argv, VALUE self)
{
    int exitcode = 1;
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
    if (pthread_create(&thread, NULL, sleep_thread_main, &arg) != 0) {
        rb_raise(rb_eRuntimeError, "pthread_create was failed");
    }

    retval = rb_funcall(block, rb_intern("call"), 0);

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
