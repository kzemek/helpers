"""This module tests S3 helper."""

__author__ = "Krzysztof Trzepla"
__copyright__ = """(C) 2016 ACK CYFRONET AGH,
This software is released under the MIT license cited in 'LICENSE.txt'."""

import os
import sys

import pytest

script_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.dirname(script_dir))
# noinspection PyUnresolvedReferences
from test_common import *
from environment import common, docker, s3
from boto.s3.connection import S3Connection, OrdinaryCallingFormat
from key_value_test_base import *
from s3_helper import S3HelperProxy


@pytest.fixture(scope='module')
def server(request):
    class Server(object):
        def __init__(self, scheme, hostname, bucket, access_key, secret_key):
            [ip, port] = hostname.split(':')
            self.scheme = scheme
            self.hostname = hostname
            self.access_key = access_key
            self.secret_key = secret_key
            self.bucket = bucket
            self.conn = S3Connection(self.access_key, self.secret_key,
                                     host=ip, port=int(port), is_secure=False,
                                     calling_format=OrdinaryCallingFormat())

        def list(self, file_id):
            bucket = self.conn.get_bucket(self.bucket, validate=False)
            return list(bucket.list(prefix=file_id + '/', delimiter='/'))

    bucket = 'data'
    result = s3.up('onedata/s3proxy', [bucket], 'storage',
                   common.generate_uid())
    [container] = result['docker_ids']

    def fin():
        docker.remove([container], force=True, volumes=True)

    request.addfinalizer(fin)

    return Server('http', result['host_name'], bucket, result['access_key'],
                  result['secret_key'])


@pytest.fixture
def helper(server):
    return S3HelperProxy(server.scheme, server.hostname, server.bucket,
                         server.access_key, server.secret_key, THREAD_NUMBER,
                         BLOCK_SIZE)
