require File.dirname(__FILE__) + '/spec_helper'

describe "TLearn" do
  before(:all) do
    FileUtils.rm_f(Dir.glob("#{TLearn::Config::WORKING_DIR}/*"))
    FileUtils.rm_f(Dir.glob("tmp/*"))
  end

  let(:out){ StringIO.new }
  let(:number_of_sweeps){ 200 }

  let(:training_data){
    [
      [{[1] * 77 => [0, 0, 0, 0, 0, 1]}],
      [{[0] * 77 => [1, 0, 0, 0, 0, 0]}]
    ]
  }

  describe 'training a neural network' do

    it "should train the tlearn neural network" do
      tlearn = TLearn::Run.new(example_config, out)
    
      tlearn.train(training_data, number_of_sweeps)
    end
  
    it "should generate a weights file that can be used for future fitness tests" do
      tlearn = TLearn::Run.new(example_config, out)

      tlearn.train(training_data, number_of_sweeps, File.dirname(__FILE__) + '/../tmp/')

      File.exists?(File.dirname(__FILE__) + '/../tmp/evaluator.wts').should be_true
    end

  end

  describe "testing fitness using trained network" do
    before(:all) do
      tlearn = TLearn::Run.new(example_config, out)
      tlearn.train(training_data, number_of_sweeps)
    end
    
    it "should take a weights file from a previous training session" do
      weights_dir = File.dirname(__FILE__) + '/../tmp/'

      test_subject = [1] * 77

      tlearn = TLearn::Run.new(example_config, out)

      rating = tlearn.fitness(test_subject, number_of_sweeps, weights_dir)

      p rating
    end

    it "should rank 77 1s with a 6" do
      test_subject = [1] * 77

      tlearn = TLearn::Run.new(example_config, out)

      rating = tlearn.fitness(test_subject, number_of_sweeps)

      p rating

      rank = rating.rindex(rating.max) + 1

      rank.should == 6
    end
    
    it "should rank 77 0s with a 1" do
      test_subject = [0] * 77

      tlearn = TLearn::Run.new(example_config, out)

      rating = tlearn.fitness(test_subject, number_of_sweeps)

      p rating

      rank = rating.rindex(rating.max) + 1

      rank.should == 1

    end
  end

end