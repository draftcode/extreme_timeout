require 'spec_helper'
require 'extreme_timeout'

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

    it 'exits the process when timeout' do
      pending "This test will terminate the whole rspec process"
      ExtremeTimeout::timeout(1) { sleep }
    end

    it 'exits the process with the exit code specified when timeout' do
      pending "This test will terminate the whole rspec process"
      ExtremeTimeout::timeout(1, 5) { sleep }
    end
  end
end
