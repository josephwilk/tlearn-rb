module TLearn
  class RunTLearn
    TLEARN_EXECUTABLE = '../bin/tlearn'
  
    EXTERNAL_RESET_FILE = '-X'
    VERIFY_OUTPUTS_ON_EACH_SWEEP = '-V'
    USE_RTRL_TEMPORALLY_RECURRENT_LEARNING = '-L'
  
    def initialize(config = {})
      @config = Config.new(config)
    end
  
    def fitness(data, number_of_sweeps = Config::DEFAULT_NUMBER_OF_SWEEPS)
      raise "Train me first!" unless network_trained?
      
      FileUtils.rm_f("#{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.output")

      @config.setup_fitness_data(data)
        
      execute_tlearn_fitness(number_of_sweeps)
    
      output = File.read("#{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.output")
      output.split("\n").map{|line| line.split("\t").map{|number| number.strip}}
    end
  
    def train(training_data, number_of_sweeps = DEFAULT_NUMBER_OF_SWEEPS)
      clear_previous_session
      
      @config.setup_config(training_data)
      @config.setup_training_data(training_data)

      execute_tlearn_train(number_of_sweeps)
          
      if training_successful?(number_of_sweeps)
        weights = File.read("#{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.#{number_of_sweeps}.wts").split("\n")
        `cp #{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.#{number_of_sweeps}.wts #{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.wts`
        weights.map{|line| line.split("\t").map{|number| number.strip}}
      else
        false
      end
    end
    
    private

    def clear_previous_session
      FileUtils.rm_f(Dir.glob("#{Config::WORKING_DIR}/*"))
    end

    def training_successful?(number_of_sweeps)
      File.exists?("#{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.#{number_of_sweeps}.wts")
    end

    def network_trained?
      File.exists?("#{Config::WORKING_DIR}/#{Config::TLEARN_NAMESPACE}.wts")
    end
  
    def execute_tlearn_fitness(number_of_sweeps)
      `cd #{Config::WORKING_DIR} && #{TLEARN_EXECUTABLE} -f #{Config::TLEARN_NAMESPACE} -l evaluator.wts -s #{number_of_sweeps} #{VERIFY_OUTPUTS_ON_EACH_SWEEP} > evaluator.output`
    end

    def execute_tlearn_train(number_of_sweeps)
      `cd #{Config::WORKING_DIR} && #{TLEARN_EXECUTABLE} -f #{Config::TLEARN_NAMESPACE} #{USE_RTRL_TEMPORALLY_RECURRENT_LEARNING} -s #{number_of_sweeps}`
    end

  end
end