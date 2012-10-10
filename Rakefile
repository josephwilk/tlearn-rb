require File.dirname(__FILE__) + '/lib/tlearn'

desc "Start a training session"
task :train do
  training_data = {[0] * 77 => [1, 0, 0, 0, 0, 0],
                   [1] * 77 => [0, 0, 0, 0, 0, 0] } 
  
  TLearn.train(training_data, sweeps = 1000)
end

task :fitness do
  TLearn.fitness([0] * 77, sweeps = 1000)
end
