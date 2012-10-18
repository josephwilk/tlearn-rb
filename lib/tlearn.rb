$:.unshift(File.dirname(__FILE__) + '/lib')

require 'tlearn/training_data'
require 'tlearn/fitness_data'
require 'tlearn/config'
require 'tlearn/run_tlearn'

require 'tlearn/run'

def tlearn_extension
  File.exists?(File.dirname(__FILE__) + '/tlearn.so') ? 'tlearn.so' : 'tlearn.bundle'
end

require tlearn_extension

module TLearn
end