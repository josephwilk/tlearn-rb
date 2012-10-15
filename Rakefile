require 'rubygems'
require 'rspec/core/rake_task'

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

SWEEPS = 1000

tlearn = TLearn::Run.new(neural_network_config)
                                          
desc "Start a training session"
task :train do
  training_data = [{[0] * 77 => [1, 0, 0, 0, 0, 0]}],
                  [{[1] * 77 => [0, 0, 0, 0, 0, 1]}]
  
  tlearn.train(training_data, sweeps = SWEEPS)
end

desc "Use trained network to evaluate an input"
task :fitness => [:train] do
  test_subject_1 = [0] * 77
  test_subject_2 = [1] * 77
  
  rating_1 = tlearn.fitness(test_subject_1, sweeps = SWEEPS)
  rating_2 = tlearn.fitness(test_subject_2, sweeps = SWEEPS)

  [[rating_1, test_subject_1], [rating_2, test_subject_2]].each do |ratings, subject|
    rank = ratings.rindex(ratings.max) + 1
    puts "rank: #{rank} => #{subject}"
    p ratings, ""
  end
end

task :install do
  `mkdir -p tmp`
  `cd tmp && wget --quiet ftp://ftp.crl.ucsd.edu/pub/neuralnets/tlearn_src/tlearn_unix.tar.gz && tar -xf tlearn_unix.tar.gz`
  
  make = File.read('tmp/tlearn/Makefile')
  make.gsub!('EXP=-DEXP_TABLE=\"$(EXP_LOC)\"', '#EXP=-DEXP_TABLE=\"$(EXP_LOC)\"')
  make.gsub!('EXP_LOC=/usr/local/lib/exp/exp_table', '#EXP_LOC=/usr/local/lib/exp/exp_table')
  File.open('tmp/tlearn/Makefile', 'w'){|f| f.write(make)}
  
  `cd tmp/tlearn && make > /dev/null 2>&1 && cp tlearn ../../bin/`
  `rm -rf tmp/tlearn`
end

RSpec::Core::RakeTask.new(:spec)
RSpec::Core::RakeTask.new(:spec_integration) do |spec|
  spec.pattern = 'spec_integration/**/*_spec.rb'
end

task :default => [:spec, :spec_integration]