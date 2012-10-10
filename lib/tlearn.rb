module TLearn
  NUMBER_OF_OUTPUT_VECTORS = 1750  
  NUMBER_OF_RESET_TIMEPOINTS = 3497
  NUMBER_OF_INPUT_VECTORS_TO_FOLLOW = 3497

  WORKING_DIR = File.dirname(__FILE__) + '/../data'
  
  def self.learn
    File.open("#{WORKING_DIR}/evaulator.cf",    "w") {|f| f.write(evaulator_config)}
    File.open("#{WORKING_DIR}/evaulator.reset", "w") {|f| f.write(reset_config)}
    File.open("#{WORKING_DIR}/evaulator.data",  "w") {|f| f.write(data)}
    File.open("#{WORKING_DIR}/evaulator.teach", "w") {|f| f.write(teach_data)}
    
    `cd #{WORKING_DIR} && ../bin/tlearn -f evaulator -V -L -X -s 1752`
    
    weights_file = "evaluator.#{NUMBER_OF_OUTPUT_VECTORS}.wts"
  end
  
  #.cf  
  def self.evaulator_config
    config = <<EOS
NODES:
nodes = 86
inputs = 77
outputs = 6
output nodes are 41-46
CONNECTIONS:
groups = 0
1-81 from 0
1-40 from i1-i77
41-46 from 1-40
1-40 from 47-86
47-86 from 1-40 = 1. & 1. fixed one-to-one
SPECIAL:
linear = 47-86
weight_limit = 1.00
selected = 1-86  
EOS
  end

  def self.data
    data_file = <<EOS
distributed
#{NUMBER_OF_INPUT_VECTORS_TO_FOLLOW}
EOS
  end

  def self.teach_data
    teach_file = <<EOS
distributed
#{NUMBER_OF_OUTPUT_VECTORS}
EOS
  end
  
  def self.reset_config
    reset_file = <<EOS
#{NUMBER_OF_RESET_TIMEPOINTS}
EOS
  end
end