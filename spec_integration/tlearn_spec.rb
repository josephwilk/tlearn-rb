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
    training_data = [{[1] * 77 => [1, 0, 0, 0, 0, 0]}],
                    [{[0] * 77 => [0, 0, 0, 0, 0, 1]}]
  
    tlearn = TLearn::Run.new(config, out)
    
    tlearn.train(training_data, number_of_sweeps)
  end
  
  it "should used the trained neural network to evaluate the fitness of candidate inputs" do
    test_subject_1 = [0] * 77
    test_subject_2 = [1] * 77
  
    tlearn = TLearn::Run.new(config, out)
  
    rating_1 = tlearn.fitness(test_subject_1, number_of_sweeps)
    rating_2 = tlearn.fitness(test_subject_2, number_of_sweeps)

    rank_1 = rating_1.rindex(rating_1.max) + 1
    rank_2 = rating_2.rindex(rating_2.max) + 1

    rank_1.should == 6
    rank_2.should == 1
  end

end