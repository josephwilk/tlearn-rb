module TLearn
  class TrainingData
    def initialize(data)
      @data_list = data
    end
    
    def reset_points
      @data_list.map{|data| data.length}
    end

    def output_data
      @data_list.reduce([]){|all_data, data| all_data + data.reduce([]){|data_list, data_hash| data_list << data_hash.values[0] }}
    end
    
    def data
      @data_list.reduce([]){|all_data, data| all_data + data.reduce([]){|data_list, data_hash| data_list << data_hash.keys[0] }}
    end

    def no_of_data_values
      @data_list.map{|data| data.length}.reduce(&:+)
    end

    def no_of_inputs
      @data_list[0][0].keys[0].length
    end
    
    def no_of_outputs
      @data_list[0][0].values[0].length
    end
      
  end
end