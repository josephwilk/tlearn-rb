require File.dirname(__FILE__) + '/lib/tlearn'

desc "Start a training session"
task :train do
  fake_data = [0] * 77
  fake_training_result = [1, 0, 0, 0, 0, 0]
  
  TLearn.train({fake_data => fake_training_result}, sweeps = 1000)
end