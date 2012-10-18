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
      it "should return a final set of iterated weights" do
        TLearnExt.fitness({'sweeps' => '1', 'file_root' => example_file_root}).should have(6).weights 
      end

      it "should return the weights as floats" do
        TLearnExt.fitness({'sweeps' => '1', 'file_root' => example_file_root}).map(&:class).uniq.should == [Float]
      end

      it "should return weights between 0 and 1" do
        TLearnExt.fitness({'sweeps' => '1', 'file_root' => example_file_root}).each do |weight|
          weight.should >= 0 && weight.should <= 1
        end
      end
    end

    context "with an invalid file root" do
      it "should ext with status 1 (an error)" do
        TLearnExt.fitness({'sweeps' => '1', 'file_root' => 'invalid file root'}).should == 1
      end
    end
  end
  
end