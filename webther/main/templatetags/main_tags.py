from django.template.defaultfilters import register


@register.filter()
def int_cmp(a_str, an_int):
    return int(a_str) == an_int
