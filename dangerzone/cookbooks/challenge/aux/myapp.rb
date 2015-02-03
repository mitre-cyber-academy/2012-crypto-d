require 'sinatra'
require 'tempfile'


class Application < Sinatra::Base
  set :app_file, __FILE__
  
  post '/' do
    temp = Tempfile.new('userInput')
    out = Tempfile.new('output')
    temp.write(request.env["rack.input"].read)
    temp.close
    content_type 'application/octet-stream'
    system("./encryptor key #{temp.path} > #{out.path}")
    send_file(out.path)
  end
end