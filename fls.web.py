from aiohttp import web

	
def init_function(argv):
    app = web.Application()
    app.router.add_static('/', '/runit', name='static') 
    return app
