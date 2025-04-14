import colorama
import uvicorn

RED = colorama.Fore.RED
RESET = colorama.Fore.RESET

def main():
    try:
        uvicorn.run('modules.app:app', reload=True, port=9200)
    except Exception as error:
        print(f"{RED}{error}{RESET}")

if __name__ == '__main__':
    main()