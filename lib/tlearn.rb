$:.unshift(File.dirname(__FILE__) + '/lib')

require 'tlearn/training_data'
require 'tlearn/fitness_data'
require 'tlearn/config'
require 'tlearn/run_tlearn'

require 'tlearn/run'

def tlearn_extension
  if File.exists?(File.dirname(__FILE__) + '/tlearn.so')
    'tlearn.so'
  elsif File.exists?(File.dirname(__FILE__) + '/tlearn.bundle')
    'tlearn.bundle'
  elsif File.exists?(File.dirname(__FILE__) + '/tlearn/tlearn.so')
    'tlearn/tlearn.so'
  elsif File.exists?(File.dirname(__FILE__) + '/tlearn/tlearn.bundle')
    'tlearn/tlearn.bundle'
  end
end

require tlearn_extension

module TLearn
end