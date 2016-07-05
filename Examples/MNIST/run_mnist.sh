unzip mnist_vec784D.txt.zip
python ../../LargeVis_run.py -input mnist_vec784D.txt -output mnist_vec2D.txt
python ../../plot.py -input mnist_vec2D.txt -label mnist_label.txt -output mnist_plot
