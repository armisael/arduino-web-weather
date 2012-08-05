

def float_or_none(something):
    try:
        return float(something)
    except TypeError:
        return None
