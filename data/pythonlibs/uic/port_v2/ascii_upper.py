import string


# A translation table for converting ASCII lower case to upper case.
_ascii_trans_table = string.maketrans(string.ascii_lowercase,
        string.ascii_uppercase)


# Convert a string to ASCII upper case irrespective of the current locale.
def ascii_upper(s):
    return s.translate(_ascii_trans_table)
