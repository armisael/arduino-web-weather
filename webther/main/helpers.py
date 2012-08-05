

def float_or_none(something):
    try:
        return float(something)
    except TypeError:
        return None


def build_navigation_entries(start_date, end_date, offset):
    """ return a list of dates, starting from start_date until end_date,
    separated by offset.
    """
    while start_date <= end_date:
        yield start_date
        start_date += offset
