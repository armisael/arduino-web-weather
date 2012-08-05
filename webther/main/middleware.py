import logging
from collections import defaultdict

from django.http import HttpResponse


logger = logging.getLogger("main.middleware")


class SpyMiddleware(object):
    """ SpyMiddleware spies at /arduino-post/ requests, and for requests that
    fail 10 or more times, returns a 200 instead of a 500, so that arduino
    is happy about life and removes the malformed log file.
    """
    MAX_FAILURES = 10
    failed_requests = defaultdict(int)

    def process_exception(self, request, exception):
        logger.exception(exception)

        if request.path != '/arduino-post/':
            return

        filename = request.GET.get('f')
        if filename is None:
            return

        self.failed_requests[filename] += 1
        if self.failed_requests[filename] >= self.MAX_FAILURES:
            return HttpResponse(content='', status=200)
