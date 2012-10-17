require 'spec_helper'

describe 'TLearnExt' do

  def tlearn_extension
    File.exists?(File.dirname(__FILE__) + '/../lib/tlearn.so') ? 'tlearn.so' : 'tlearn.bundle'
  end

  def example_file_root
    File.expand_path(File.dirname(__FILE__) + '/../data')  + '/evaluator'
  end

  before(:all) do
    require tlearn_extension
  end

  it "should not segfault" do
    TLearnExt.train({})   rescue{}
    TLearnExt.fitness({}) rescue{}
  end

  describe ".train" do
    context "with a valid file root" do
      it "should exit with status 0 (sucess)" do
        TLearnExt.train({'sweeps' => '1', 'file_root' => example_file_root}).should == 0
      end
    end

    context "with an invalid file root" do
      it "should ext with status 1 (an error)" do
        TLearnExt.train({'sweeps' => '1', 'file_root' => 'invalid file root'}).should == 1
      end
    end
  end

  describe ".fitness" do
    context "with a valid file root" do
      it "should exit with status 0 (sucess)" do
        TLearnExt.fitness({'sweeps' => '1', 'file_root' => example_file_root}).should == 0
      end
    end

    context "with an invalid file root" do
      it "should ext with status 1 (an error)" do
        TLearnExt.fitness({'sweeps' => '1', 'file_root' => 'invalid file root'}).should == 1
      end
    end
  end
  
end