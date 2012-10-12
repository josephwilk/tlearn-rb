module TLearn
  class TrainingData
    def initialize(data)
      @data_list = data
    end
    
    def reset_points
      @data_list.map{|data| data.length}
    end

    def output_data
      []
    end
    
    def data
      []
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
      
    def flattern_training_data(training_data_list)
      
    end
    
  end
end