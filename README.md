TLearn Ruby
=========

[![Build Status](https://secure.travis-ci.org/josephwilk/tlearn-rb.png)](http://travis-ci.org/josephwilk/tlearn-rb)

Ruby helping make tlearn neural network simulator sane(ish) (http://crl.ucsd.edu/innate/tlearn.html)

To see an example run:

<pre><code>rake example:train
rake example:fitness
</code></pre>

Seriously? Just use FANN!
=========

TLearn supports recurrent networks (http://en.wikipedia.org/wiki/Recurrent_neural_network), FANN does not. Recurrent networks maintain state, enabling the context of previous inputs to effect further outputs. 

While there have been attempts to add recurrent networks to FANN (http://leenissen.dk/fann/forum/viewtopic.php?t=47) these are still sitting on a dead branch that was never merged into master.

Installing TLearn
=========

<pre><code>bundle install tlearn</code></pre>

Usage
=========

```ruby
require 'tlearn'

tlearn = TLearn.new({:nodes       => {:number_of_nodes => 86,
                     :output_nodes => 41..46},
                     :connections => [{1..81   => 0},
                                      {1..40   => 'i1-i77'},
                                      {41..46  => '1-40'},
                                      {1..40   =>  '47-86'},
                                      {47..86  => ' 1-40 = 1. & 1. fixed one-to-one'}],
                     :special      => {:linear => '47-86',
                     :weight_limit => 1.00,
                     :selected     => '1-86'}})

tlearn = TLearn.new(neural_network_config)
  
training_data = [{[0] * 77 => [1, 0, 0, 0, 0, 0]}],
                [{[1] * 77 => [0, 0, 0, 0, 0, 1]}]
  
tlearn.train(training_data, sweeps = 200)

tlearn.fitness([0] * 77, sweeps = 200)
# => ["0.016", "0.013", "0.022", "0.020", "0.463", "0.467"]
```

License
=========

(The MIT License)

Copyright (c) 2012 Joseph Wilk

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.