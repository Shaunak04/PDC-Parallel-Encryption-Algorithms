import os
import os.path
from os import listdir
from os.path import isfile, join
import boto3
from botocore.client import Config
import sys
import subprocess
from keys import keys


ACCESS_KEY_ID = keys["ACCESS_KEY_ID"]
ACCESS_SECRET_KEY = keys["ACCESS_SECRET_KEY"]
BUCKET_NAME = keys["BUCKET_NAME"]
file_name = keys["file_name"]
decrypt_file = keys["decrypt_file"]

command = sys.argv[1]


s3 = boto3.resource('s3', 
aws_access_key_id = ACCESS_KEY_ID,
aws_secret_access_key = ACCESS_SECRET_KEY,
config = Config(signature_version='s3v4'))

s3s = boto3.client('s3',
        aws_access_key_id = ACCESS_KEY_ID,
        aws_secret_access_key = ACCESS_SECRET_KEY,
        config = Config(signature_version='s3v4'))

if(command=="upload"):
    data = open(file_name, 'rb')
    s3.Bucket(BUCKET_NAME).put_object(Key=file_name, Body = data)
    print("Uploaded ",sys.argv[2], " to the bucket successfully")

elif(command=="decrypt"):
    s3s.download_file(BUCKET_NAME,file_name,decrypt_file)

elif(command=="compile"):
    subprocess.call("make", shell=True)
    subprocess.call("./test test.txt", shell=True)
    print("Compiled C files successfully")

elif(command=="download"):
    print("Decrypted file downloaded in ./decrypted folder/")

elif(command=="list"):
    session = boto3.Session( 
         aws_access_key_id=ACCESS_KEY_ID, 
         aws_secret_access_key=ACCESS_SECRET_KEY)

    reads3 = session.resource('s3')
    my_bucket = reads3.Bucket(BUCKET_NAME)
    for my_bucket_object in my_bucket.objects.all():
        print(my_bucket_object.key)