# Generating data w/ 2 classes

Please specify the 2 classes in line 189 & 190 of download_and_convert_visualwakewords.py

It's not specified in download_and_convert_data.py as usual

In download_and_convert_visualwakewords_lib.py, 2 functions are added: dues_2class_create_visual_wakeword_annotations, dues_2class_filter_annotations

They're used to generate data w/ 2 classes

Use the following command to run

`python download_and_convert_data.py --dataset_name=visualwakewords --dataset_dir=/tmp/visualwakewords`

----------------
# Run w/ coco2017

Files w/ coco2017 in their name are for the coco 2017 dataset

I didn't handle the downloading process in the code, so please download & unzip train2017, val2017, annotationstrainval2017 first

Use the following command to run

python download_and_convert_data_coco2017.py --dataset_name=visualwakewords --dataset_dir=/tmp/visualwakewords
