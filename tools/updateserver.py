from flask import Flask, make_response, request, current_app, jsonify, json
from functools import update_wrapper
from os import path

FILEDIR = ''

def corsDecorator(f):
    def new_func(*args, **kwargs):
        resp = make_response(f(*args, **kwargs))

        resp.cache_control.no_cache = True                   # Turn off caching
        resp.headers['Access-Control-Allow-Origin'] = '*'    # Add header to allow CORS

        return resp
    return update_wrapper(new_func, f)


app = Flask(__name__, static_folder=FILEDIR)



@app.route('/')
def home():
    return "RGBWW Update Server" 
	
@app.route('/version.json')
@corsDecorator
def versioninfo():
	rom = 0
	webapp = 0
	if path.isfile(path.join(FILEDIR, "rom0.bin")):
		rom = {}
		rom["version"] = "unknown"
		rom["url"] = "http://"+request.host+"/rom0.bin"
		
	if path.isfile(path.join(FILEDIR, "index.html.gz")) and path.isfile(path.join(FILEDIR, "init.html.gz")) and \
			path.isfile(path.join(FILEDIR, "app.min.js.gz")) and path.isfile(path.join(FILEDIR, "app.min.css.gz")):
		webapp = {}
		webapp["version"] = "unknown"
		webapp["url"] = ["http://"+request.host+"/index.html.gz",
		"http://"+request.host+"/init.html.gz",
		"http://"+request.host+"/app.min.css.gz",
		"http://"+request.host+"/app.min.js.gz"]
	if rom is not 0 and webapp is not 0:
		resp = { "rom": rom, "webapp": webapp}
	elif rom is not 0:
		resp = {"rom" : rom }
	elif webapp is not 0:
		resp = { "webapp" : webapp }
	else:
		resp = {}
	return json.dumps(resp)

@app.route('/<path:path>')
@corsDecorator
def static_proxy(path):
  return app.send_static_file(path)

if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0", port=80)