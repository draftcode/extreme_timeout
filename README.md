# ExtremeTimeout

Do it in 40 seconds, or die.

    require 'extreme_timeout'
    ExtremeTimeout::timeout(40, 42) do
      do_something
    end

If `do_something` doesn't return in 40 seconds, ExtremeTimeout forcibly
terminates the interpreter with exitcode 42.

## Why do I need this?

Let us assume that we want to set a timeout for some work. You might write a
code like this:

    require 'timeout'
    Timeout::timeout(40) do
      do_something
    end

It is okay in the normal case. But sometimes you have to get along with a
C-extension that holds the GVL for a long time. In this case, the code above
doesn't work as intended. The `timeout` library uses a ruby level thread to
accomplish its task, and there is no chance to get a control to that thread
under the situation that some C-extension is working with holding the GVL.

## Solution

Run a native thread that is not controlled by the interpreter. Since it is not
controlled by the interpreter, it can work freely without considering the GVL.
`ExtremeTimeout` creates a new native thread and sleeps for some seconds and
terminates the process.

## Usage

You can use this almost same as `timeout` library. Instead of the exception
class, you should specify the exitcode.

    ExtremeTimeout::timeout(timeouse_sec, exitcode=1, &block)
