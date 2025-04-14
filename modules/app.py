import os
import fastapi
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles

app = fastapi.FastAPI()

PATH_TEMPLATES = os.path.join(os.path.dirname(__file__), 'templates')
html_templates = Jinja2Templates(directory=PATH_TEMPLATES)

PATH_STATIC = os.path.join(os.path.dirname(__file__), 'css')
app.mount("/static", StaticFiles(directory=PATH_STATIC), name="static")