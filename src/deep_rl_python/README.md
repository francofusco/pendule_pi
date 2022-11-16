 ## Cart-Pole minimum version
# deepFish


The main scripts are:
* deepRl/generate_data.py for launching deep rl trainings and generating .zip and .csv to be used in the plots
* deepRl/plots.py for plotting data
* deepRl/src/theta_x_experiment.py for plotting dependance of x on x_dot and theta on theta_dot
* motor_identification/EJPH.py for identification of physical parameters in appendix
* q_learning/plot_paper_qlearning.ipynb for q-learning plot
* learning_lli folder contains scripts when launching lli c++ interface on raspberry pi4, as alternative you can launch transfer_rpi.py on PC and python code tcp_interface.py on raspberry

# working version of code is in master branch.

fr: le code de l'apprentissage par renforcement pour la pendule inversé
## Install
pip3 install -r requirements.txt