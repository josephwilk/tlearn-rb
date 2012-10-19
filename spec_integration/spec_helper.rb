require 'rspec'

require File.dirname(__FILE__) + '/../lib/tlearn'


def example_config
  {:nodes       => {:nodes => 86},
                    :connections => [{1..81   => '0'},
                                     {1..40   => 'i1-i77'},
                                     {41..46  => '1-40'},
                                     {1..40   =>  '47-86'},
                                     {47..86  => ' 1-40 = 1. & 1. fixed one-to-one'}],
                    :special      => {:linear => '47-86',
                    :weight_limit => '1.00',
                    :selected     => '1-86'}}
end

at_exit do
  FileUtils.rm_f(Dir.glob("#{TLearn::Config::WORKING_DIR}/*"))
end