from uic.exceptions import NoSuchWidgetError


def invoke(driver):
    """ Invoke the given command line driver.  Return the exit status to be
    passed back to the parent process.
    """

    exit_status = 1

    try:
        exit_status = driver.invoke()

    except IOError, e:
        driver.on_IOError(e)

    except SyntaxError, e:
        driver.on_SyntaxError(e)

    except NoSuchWidgetError, e:
        driver.on_NoSuchWidgetError(e)

    except Exception, e:
        driver.on_Exception(e)

    return exit_status
