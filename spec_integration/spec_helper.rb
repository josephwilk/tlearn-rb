require 'rspec'

require File.dirname(__FILE__) + '/../lib/tlearn'


def example_config
  {:number_of_nodes => 86,
   :output_nodes    => 41..46,
   :linear          => 47..86,
   :weight_limit    => 1.00,
   :selected        => 1..86,
   :connections     => [{1..81   => 0},
                        {1..40   => :i1..:i77},
                        {41..46  => 1..40},
                        {1..40   => 47..86},
                        {47..86  => [1..40, {:max => 1.0, :min => 1.0}, :fixed, :one_to_one]}]}
end

at_exit do
  FileUtils.rm_f(Dir.glob("#{TLearn::Config::WORKING_DIR}/*")) unless ENV['KEEP_DATA']
  FileUtils.rm_f(Dir.glob("tmp/test/*")) unless ENV['KEEP_DATA']
end