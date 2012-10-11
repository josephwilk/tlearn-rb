require File.dirname(__FILE__) + '/lib/tlearn'

neural_network_config = {:nodes       => {:nodes => 86},
                         :connections => {'1-81' => '0',
                                          '1-40' => 'i1-i77',
                                          '41-46' => '1-40',
                                          '1-40' =>  '47-86',
                                          '47-86' => ' 1-40 = 1. & 1. fixed one-to-one'},
                         :special     => {:linear => '47-86',
                                          :weight_limit => '1.00',
                                          :selected => '1-86'}}

desc "Start a training session"
task :train do
  tlearn = TLearn.new(neural_network_config)
  
  training_data = {[0] * 77 => [1, 0, 0, 0, 0, 0],
                   [1] * 77 => [0, 0, 0, 0, 0, 1]} 
  
  tlearn.train(training_data, sweeps = 1000)
end

task :fitness do
  test_subject = [1] * 77
  
  tlearn = TLearn.new(neural_network_config)
  
  ratings = tlearn.fitness(test_subject, sweeps = 1000)

  p ratings, ""
  
  rank = ratings.rindex(ratings.max) + 1

  puts "rank: #{rank} => #{test_subject}"
end