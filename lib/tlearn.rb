module TLearn
  number_of_output_vectors = 1750  
  
  def learn
    `./tlearn -f evaluator -V -L -X -s 1752`
    weights_file = "evaluator.#{number_of_output_vectors}.wts"
  end
  
  #.cf  
  evaulator_config = <<EOS
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

  #.data
  number_of_input_vectors_to_follow = 3497
  data = [[]]
  data_file = = <<EOS
distributed
#{number_of_input_vectors_to_follow}
EOS

  #.teach
  teach_file = <<EOS
distributed
#{number_of_output_vectors}
EOS

  #.reset
  number_of_reset_timepoints = 3497
  reset_file = <<EOS
#{number_of_reset_timepoints}
EOS
end