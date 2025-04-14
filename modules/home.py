import httpx

from .app import app, html_templates
from fastapi import Request
from fastapi.responses import JSONResponse

ESP8266_URL = "http://192.168.1.100/update_pid"

@app.get('/')
async def homepage(request: Request):
    return html_templates.TemplateResponse(
        request=request,
        name='base.html'
    )


@app.post("/pid")
async def set_pid(request: Request):
    form_data = await request.form()

    p_value = float(form_data.get('P'))
    i_value = float(form_data.get('I'))
    d_value = float(form_data.get('D'))

    print(f"P: {p_value}, I: {i_value}, D: {d_value}")

    pid_values = {
        "P": p_value,
        "I": i_value,
        "D": d_value
    }

    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(ESP8266_URL, json=pid_values)
            
            if response.status_code == 200:
                print("Данные успешно отправлены на ESP8266.")
            else:
                print(f"Ошибка при отправке данных: {response.status_code}")
        except Exception as e:
            print(f"Ошибка при подключении к ESP8266: {e}")

    return JSONResponse(content=pid_values)