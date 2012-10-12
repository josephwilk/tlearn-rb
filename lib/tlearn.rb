$:.unshift(File.dirname(__FILE__) + '/lib')

require 'tlearn/training_data'
require 'tlearn/fitness_data'
require 'tlearn/config'
require 'tlearn/run_tlearn'

module TLearn
  class Run

    def initialize(config)
      @config = config
    end

    def train(data, number_of_sweeps = nil)
      run_tlearn = RunTLearn.new(@config)

      run_tlearn.train(TrainingData.new(data), number_of_sweeps)
    end

    def fitness(data, number_of_sweeps = nil)
      run_tlearn = RunTLearn.new(@config)

      run_tlearn.fitness(FitnessData.new(data), number_of_sweeps)
    end
  
  end
end