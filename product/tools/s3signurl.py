#!/usr/bin/env python
import argparse
import sys

from boto.s3.connection import S3Connection

parser = argparse.ArgumentParser(description='generates signed url for s3 object')
parser.add_argument('-b', '--bucket', dest='bucket', help='S3 bucket containing the file')
parser.add_argument('-p', '--path', dest='path', help='Path to the file (relative to the bucket)')
parser.add_argument('-a', '--access-key', dest='access_key', help='Your AWS Access Key ID')
parser.add_argument('-s', '--secret-key', dest='secret_key', help='Your AWS secret key')
parser.add_argument('--no-https', dest='https', action='store_false', default=True, help='Disable serving over HTTPS')
parser.add_argument('--expiry', dest='expiry', default='31557600', help='Expiration time, in seconds (defaults to one year)')

args = parser.parse_args()

def sign(bucket, path, access_key, secret_key, https, expiry):
    c = S3Connection(access_key, secret_key)
    return c.generate_url(
        expires_in=long(expiry),
        method='GET',
        bucket=bucket,
        key=path,
        query_auth=True,
        force_http=(not https)
    )
    
for opt in ('bucket', 'path', 'access_key', 'secret_key'):
    assert args.__dict__.get(opt), '%s is not optional' % opt
    
print sign(
    bucket=args.bucket,
    path=args.path,
    access_key=args.access_key,
    secret_key=args.secret_key,
    https=args.https,
    expiry=long(args.expiry)
    )

sys.exit(0)
