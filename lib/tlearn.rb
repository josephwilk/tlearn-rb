class TLearn
  NUMBER_OF_RESET_TIMEPOINTS = 3497
  DEFAULT_NUMBER_OF_SWEEPS = 1333000

  WORKING_DIR = File.dirname(__FILE__) + '/../data'
  TLEARN_EXECUTABLE = '../bin/tlearn'
  TLEARN_NAMESPACE = 'evaluator'
  
  EXTERNAL_RESET_FILE = '-X'
  VERIFY_OUTPUTS_ON_EACH_SWEEP = '-V'
  USE_RTRL_TEMPORALLY_RECURRENT_LEARNING = '-L'
  
  
  def initialize(config = {})
    @connections_config = config[:connections]
    @special_config     = config[:special]
    @nodes_config       = config[:nodes] 
  end
  
  def fitness(data, number_of_sweeps = DEFAULT_NUMBER_OF_SWEEPS)
    raise "Train me first!" unless network_trained?
      
    FileUtils.rm_f("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.output")

    setup_fitness_data({data => []})
        
    execute_tlearn_fitness(number_of_sweeps)
    
    output = File.read("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.output")
    output = output.split("\n").map{|line| line.split("\t").map{|number| number.strip}}
    output.each{|o| p o}
    p
    output[-1]
  end
  
  def train(training_data, number_of_sweeps = DEFAULT_NUMBER_OF_SWEEPS)
    clear_previous_session
      
    setup_config(training_data)
    setup_training_data(training_data)

    execute_tlearn_train(number_of_sweeps)
          
    if training_successful?(number_of_sweeps)
      puts File.read("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.#{number_of_sweeps}.wts").split("\n")
      `cp #{WORKING_DIR}/#{TLEARN_NAMESPACE}.#{number_of_sweeps}.wts #{WORKING_DIR}/#{TLEARN_NAMESPACE}.wts`
    else
      puts("[Error] Training failed") 
    end
  end
    
  private

  def clear_previous_session
    FileUtils.rm_f(Dir.glob("#{WORKING_DIR}/*"))
  end

  def training_successful?(number_of_sweeps)
    File.exists?("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.#{number_of_sweeps}.wts")
  end

  def network_trained?
    File.exists?("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.wts")
  end
  
  def execute_tlearn_fitness(number_of_sweeps)
    `cd #{WORKING_DIR} && #{TLEARN_EXECUTABLE} -f #{TLEARN_NAMESPACE} -l evaluator.wts -s #{number_of_sweeps} #{VERIFY_OUTPUTS_ON_EACH_SWEEP} > evaluator.output`
  end

  def execute_tlearn_train(number_of_sweeps)
    `cd #{WORKING_DIR} && #{TLEARN_EXECUTABLE} -f #{TLEARN_NAMESPACE} #{USE_RTRL_TEMPORALLY_RECURRENT_LEARNING} -s #{number_of_sweeps}`
  end

  def setup_config(training_data)
    File.open("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.cf",    "w") {|f| f.write(evaulator_config(training_data))}
  end

  def setup_fitness_data(data)
    File.open("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.data",  "w") {|f| f.write(build_data(data))}
  end

  def setup_training_data(training_data)
    #File.open("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.reset", "w") {|f| f.write(reset_config)}
    File.open("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.data",  "w") {|f| f.write(build_data(training_data))}
    File.open("#{WORKING_DIR}/#{TLEARN_NAMESPACE}.teach", "w") {|f| f.write(build_teach_data(training_data))}
  end
  
  def evaulator_config(training_data)
    no_of_inputs  = training_data.keys[0].length
    no_of_outputs = training_data.values[0].length
      
    nodes_config = {
      :nodes => @nodes_config[:nodes],
      :inputs => no_of_inputs,
      :outputs => no_of_outputs,
      :output_nodes => '41-46'
    }
    @connections_config[:groups] = 0
  
    output_nodes = nodes_config.delete(:output_nodes)
    node_config_strings = nodes_config.map{|key,value| "#{key.to_s.gsub('_',' ')} = #{value}" }
    node_config_strings << "output nodes are #{output_nodes}"

    groups = @connections_config.delete(:groups)
    connection_config_strings = @connections_config.map{|key,value| "#{key} from #{value}" }
    connection_config_strings =  ["groups = #{groups}"] + connection_config_strings

    config = <<EOS
NODES:
#{node_config_strings.join("\n")}
CONNECTIONS:
#{connection_config_strings.join("\n")}
SPECIAL:
#{@special_config.map{|key,value| "#{key} = #{value}" }.join("\n")}
EOS
  end

  def build_data(training_data)
    number_of_input_vectors_to_follow = training_data.values.length
      
    data_file = <<EOS
distributed
#{number_of_input_vectors_to_follow}
#{training_data.keys.map{|key| key.join(" ")}.join("\n")}
EOS
  end

  def build_teach_data(training_data)
    teach_file = <<EOS
distributed
#{training_data.values.length}
#{training_data.values.map{|value| value.join(" ")}.join("\n")}
EOS
  end
  
  def reset_config
    reset_file = <<EOS
#{NUMBER_OF_RESET_TIMEPOINTS}
EOS
  end
    
  def number_of_outputs(training_data)
    training_data.values[0].length
  end
  
end