require 'spec_helper'

describe "TLearn" do
  let(:config) {
    {:nodes       => {:nodes => 86},
    :connections => {'1-81' => '0',
                     '1-40' => 'i1-i77',
                     '41-46' => '1-40',
                     '1-40' =>  '47-86',
                     '47-86' => ' 1-40 = 1. & 1. fixed one-to-one'},
    :special     => {:linear => '47-86',
                     :weight_limit => '1.00',
                     :selected => '1-86'}}
  }
  
  let(:out){ StringIO.new }
  
  it "should train the tlearn neural network" do
    training_data = [{[1] * 77 => [1, 0, 0, 0, 0, 0]}, {[1] * 77 => [1, 0, 0, 0, 0, 0]}],
                    [{[0] * 77 => [0, 0, 0, 0, 0, 1]}]
  
    tlearn = TLearn::Run.new(config, out)
    
    tlearn.train(training_data, sweeps = 1)
  end
  
  it "should provide a output from the neural network" do
    test_subject_1 = [0] * 77
    test_subject_2 = [1] * 77
  
    tlearn = TLearn::Run.new(config, out)
  
    rating_1 = tlearn.fitness(test_subject_1, sweeps = 1)
    rating_2 = tlearn.fitness(test_subject_2, sweeps = 1)

    [[rating_1, test_subject_1], [rating_2, test_subject_2]].each do |ratings, subject|
      rank = ratings.rindex(ratings.max) + 1
      puts "rank: #{rank} => #{subject}"
      p ratings, ""
    end
  end

end