require 'spec_helper'

describe ExtremeTimeout do
  describe '.timeout' do
    it 'checks timeout argument' do
      expect { ExtremeTimeout::timeout("10") { } }.to raise_error(ArgumentError)
      expect { ExtremeTimeout::timeout() { } }.to raise_error(ArgumentError)
    end

    it 'checks exitcode argument' do
      expect { ExtremeTimeout::timeout(10, "2") { } }.to raise_error(ArgumentError)
    end

    it 'does not execute the block when it raise an ArgumentError' do
      executed = false
      expect { ExtremeTimeout::timeout('10') { executed = true } }.to raise_error(ArgumentError)
      expect(executed).to be_false
    end

    it 'executes block' do
      executed = false
      ExtremeTimeout::timeout(10) { executed = true }
      expect(executed).to be_true
    end

    it 'returns the block return value' do
      expect(ExtremeTimeout::timeout(10) { 42 }).to eq(42)
    end

    it 'raises ArgumentError if no block given' do
      expect { ExtremeTimeout::timeout(3) }.to raise_error(ArgumentError)
    end

    def redirectio_fork(&block)
      stdout_r, stdout_w = IO.pipe
      stderr_r, stderr_w = IO.pipe
      pid = fork do
        $stdout.reopen(stdout_w)
        $stderr.reopen(stderr_w)
        block.call
      end
      stdout_w.close
      stderr_w.close
      return pid, stdout_r, stderr_r
    end

    it 'exits the process when timeout' do
      pid, = redirectio_fork do
        ExtremeTimeout::timeout(1) { sleep }
      end
      Timeout.timeout(3) do
        pid, status = Process.waitpid2(pid)
        expect(status.exitstatus).to eq(1)
      end
    end

    it 'exits the process with the exit code specified when timeout' do
      pid, = redirectio_fork do
        ExtremeTimeout::timeout(1, 5) { sleep }
      end
      Timeout.timeout(3) do
        pid, status = Process.waitpid2(pid)
        expect(status.exitstatus).to eq(5)
      end
    end

    it 'outputs timeout message to stderr' do
      pid, stdout, stderr = redirectio_fork do
        ExtremeTimeout::timeout(1) { sleep }
      end
      Timeout.timeout(3) do
        Process.waitpid(pid)
        expect(stderr.read).to start_with("Process exits(ExtremeTimeout::timeout)\n")
      end
    end

    it 'discards timeout if an exception was raised' do
      begin
        ExtremeTimeout::timeout(1) { raise Exception.new }
        fail 'An exception should be raised'
      rescue Exception
        sleep(5)
      end
    end
  end
end
