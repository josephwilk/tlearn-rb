require File.dirname(__FILE__) + '/spec_helper'

describe 'TLearnExt' do
  let(:out){ StringIO.new }
  let(:number_of_sweeps){ 200 }

  before(:all) do
    training_data = [{[1] * 77 => [0, 0, 0, 0, 0, 1]}],
                    [{[0] * 77 => [1, 0, 0, 0, 0, 0]}]

    tlearn = TLearn::Run.new(example_config, out)

    tlearn.train(training_data, number_of_sweeps)
  end

  def example_file_root
    File.expand_path(File.dirname(__FILE__) + '/../data')  + '/evaluator'
  end

  it "should not segfault" do
    TLearnExt.train({})   rescue{}
    TLearnExt.fitness({}) rescue{}
  end

  describe ".train" do
    context "with a valid file root" do
      it "should exit with status 0 (sucess)" do
        TLearnExt.train({:sweeps => 1, :file_root => example_file_root}).should == 0
      end
    end

    context "with an invalid file root" do
      it "should ext with status 1 (an error)" do
        TLearnExt.train({:sweeps => 1, :file_root => 'data/no config exists'}).should == 1
      end
    end
  end

  describe ".fitness" do
    context "with a valid file root" do
      it "should not return a status " do
        TLearnExt.fitness({:sweeps => 1, :file_root => example_file_root}).should_not == 1
      end
      
      it "should return a final set of iterated weights" do
        TLearnExt.fitness({:sweeps => 1, :file_root => example_file_root}).should have(6).weights
      end

      it "should return the weights as floats" do
        TLearnExt.fitness({:sweeps => 1, :file_root => example_file_root}).map(&:class).uniq.should == [Float]
      end

      it "should return weights between 0 and 1" do
        TLearnExt.fitness({:sweeps => 1, :file_root => example_file_root}).each do |weight|
          weight.should >= 0 && weight.should <= 1
        end
      end
    end

    context "with an invalid file root" do
      it "should ext with status 1 (an error)" do
        TLearnExt.fitness({:sweeps => 1, :file_root => 'no config exists'}).should == 1
      end
    end
  end
  
end