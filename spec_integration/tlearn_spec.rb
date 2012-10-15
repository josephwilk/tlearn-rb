require 'spec_helper'

describe "TLearn" do
  let(:config) {
    {:nodes       => {:nodes => 86},
    :connections => [{'1-81' => '0'},
                     {'1-40' => 'i1-i77'},
                     {'41-46' => '1-40'},
                     {'1-40' =>  '47-86'},
                     {'47-86' => ' 1-40 = 1. & 1. fixed one-to-one'}],
    :special     => {:linear => '47-86',
                     :weight_limit => '1.00',
                     :selected => '1-86'}}
  }
  
  let(:out){ StringIO.new }
  
  let(:number_of_sweeps){ 100 }
  
  it "should train the tlearn neural network" do
    training_data = [{[1] * 77 => [0, 0, 0, 0, 0, 1]}],
                    [{[0] * 77 => [1, 0, 0, 0, 0, 0]}]
  
    tlearn = TLearn::Run.new(config, out)
    
    tlearn.train(training_data, number_of_sweeps)
  end
  
  describe "testing fitness using trained network" do
    it "should rank 77 1s with a 6" do
      test_subject = [1] * 77
  
      tlearn = TLearn::Run.new(config, out)
  
      rating = tlearn.fitness(test_subject, number_of_sweeps)

      p rating

      rank = rating.rindex(rating.max) + 1

      rank.should == 6
    end
    
    it "should rank 77 0s with a 1" do
      test_subject = [0] * 77
  
      tlearn = TLearn::Run.new(config, out)
  
      rating = tlearn.fitness(test_subject, number_of_sweeps)

      p rating

      rank = rating.rindex(rating.max) + 1

      rank.should == 1
    
    end
  end

end