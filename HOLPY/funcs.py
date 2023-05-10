



class Error:
    ERRORS = {
        "INVALID_TOKEN_TYPE": "Invalid token type.",
        "FAILED_TO_READ_INPUT_FILE": "Failed to read input file.",
        "REACHED_END_OF_INPUT_FILE": "Reached end of input file.",
        "FAILED_TO_READ_NEXT_LINE": "Failed to read next line from input file."
    }

    @staticmethod
    def get_error(error_type):
        return Error.ERRORS.get(error_type, "Unknown error.")





def get_token(type):
    bf = [""] * 200
    d = 0.0
    i = 0
    
    global p
    global fin
    
    if p is None or p[0] == "":
        while True:
            fin_line = fin.readline()
            if fin_line[0] != "*":
                bf = fin_line
            if not fin_line or fin_line[0] != "*":
                break
        if bf[0] and not fin_line:
            p = bf
        else:
            raise Error

    if type == 'DBL':
        d = float(p)
        while p and not (p.isdigit() or p.startswith(".") or p.startswith("+") or p.startswith("-")):
            p = p[1:]
        return d
    elif type == 'INT':
        i = int(p)
        while p and not (p.isdigit() or p.startswith(".") or p.startswith("+") or p.startswith("-")):
            p = p[1:]
        return i
    else:
        return None

