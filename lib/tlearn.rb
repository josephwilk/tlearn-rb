module TLearn
  NUMBER_OF_OUTPUT_VECTORS = 1750  
  NUMBER_OF_RESET_TIMEPOINTS = 3497

  WORKING_DIR = File.dirname(__FILE__) + '/../data'
  
  class << self
  
    def train(training_data)
      setup_config(training_data)
      setup_data(training_data)
      execute_tlearn

      puts("[Error] Training failed") unless training_successful?
    end
    
    private

    def training_successful?
      File.exists?("#{WORKING_DIR}/evaluator.#{NUMBER_OF_OUTPUT_VECTORS}.wts")
    end

    def execute_tlearn
      `cd #{WORKING_DIR} && ../bin/tlearn -f evaulator -V -L -X -s 1752`
    end

    def setup_config(training_data)
      File.open("#{WORKING_DIR}/evaulator.cf",    "w") {|f| f.write(evaulator_config(training_data))}
    end

    def setup_data(training_data)
      File.open("#{WORKING_DIR}/evaulator.reset", "w") {|f| f.write(reset_config)}
      File.open("#{WORKING_DIR}/evaulator.data",  "w") {|f| f.write(build_data(training_data))}
      File.open("#{WORKING_DIR}/evaulator.teach", "w") {|f| f.write(build_teach_data(training_data))}
    end
  
    def evaulator_config(training_data)
      no_of_inputs  = training_data.keys[0].length
      no_of_outputs = training_data.values[0].length
      
      nodes_config = {
        :nodes => 86,
        :inputs => no_of_inputs,
        :outputs => no_of_outputs,
        :output_nodes => '41-46'
      }
      connections_config = {
        :groups => 0,
        '1-81' => '0',
        '1-40' => 'i1-i77',
        '41-46' => '1-40',
        '1-40' =>  '47-86',
        '47-86' => ' 1-40 = 1. & 1. fixed one-to-one'
      }
      special_config = {
        :linear => '47-86',
        :weight_limit => '1.00',
        :selected => '1-86'
      }

      output_nodes = nodes_config.delete(:output_nodes)
      node_config_strings = nodes_config.map{|key,value| "#{key.to_s.gsub('_',' ')} = #{value}" }
      node_config_strings << "output nodes are #{output_nodes}"

      groups = connections_config.delete(:groups)
      connection_config_strings = connections_config.map{|key,value| "#{key} from #{value}" }
      connection_config_strings =  ["groups = #{groups}"] + connection_config_strings

      config = <<EOS
NODES:
#{node_config_strings.join("\n")}
CONNECTIONS:
#{connection_config_strings.join("\n")}
SPECIAL:
#{special_config.map{|key,value| "#{key} = #{value}" }.join("\n")}
EOS
    end

    def build_data(training_data)
      number_of_input_vectors_to_follow = training_data.values.length
      
      data_file = <<EOS
distributed
#{number_of_input_vectors_to_follow}
#{training_data.keys.map{|key| key.join("")}.join("\n")}
EOS
    end

    def build_teach_data(training_data)
      teach_file = <<EOS
distributed
#{NUMBER_OF_OUTPUT_VECTORS}
#{training_data.values.join("\n")}
EOS
    end
  
    def reset_config
      reset_file = <<EOS
#{NUMBER_OF_RESET_TIMEPOINTS}
EOS
    end
  
  end
  
end