Gem::Specification.new do |s|
  s.name = 'tlearn'
  s.version = '0.0.1'
  s.summary = 'ruby bindings for tlearn'
  s.authors = ["Joseph Wilk"]
  s.email = ["joe@josephwilk.net"]
 
  s.files = Dir.glob('lib/**/*.rb') +
            Dir.glob('ext/**/*.{c,h,rb}')
  s.extensions = ['ext/extconf.rb']
  s.executables = ['tlearn']
end