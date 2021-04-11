'''
getData.py

Description:
Scans for the distance data being sent in packages over UART and creates an
xyz file based on the angle each distance was recorded. No error checking is
performed on inputs so as to not complicated the programming flowchart
documentation.

Written by: Elston Almeida
Year: 2020
'''

import serial
import math as m

if __name__ == "__main__":

    print("2DX4 - Final Project")
    print("Data Collection")

    # Get parameters for data collection
    nData    = input("Enter the # of data points per 360 degrees (up to 512): ")
    N        = input("Enter the # of samples in total: ")
    deltaX   = input("Enter the constant change in X per sample (X is in mm): ")
    COM_PORT = input("Enter the COM PORT NUMBER of MSP432E401Y (eg. Enter 5 for COM5): ")
    
    nData = int(nData)
    N = int(N)
    deltaX = int(deltaX)

    # Connect to the specified com port
    s = serial.Serial("COM"+COM_PORT, 115200)
    print("Opening: " + s.name)

    data = []
    deltaTheta = 360/nData
    x = 0
    cont = True


    print("Current Settings: ")
    print("deltaX: {}".format(deltaX))
    print("Samples: {}".format(N))
    print("--------------------------")

    try:
        # Open Output file
        with open('plotFile.xyz','w') as f:
            # For X samples
            for p in range(N):
                # Read current data points
                for i in range(nData):  #Stall for data
                    print("Waiting for Data...")
                    x1 = int(s.read(4))
                    data.append(x1)
                    print("{}/{}: Data Recieved: {}".format(i+1,nData,x1))
                print(data)
                # Convert list of data points recieved output to xyz
                for j in range(len(data)):
                    y = data[j]*m.cos(m.radians(deltaTheta*(j+1)))
                    z = data[j]*m.sin(m.radians(deltaTheta*(j+1)))
                    print("x: {} , y: {} , z: {}".format(x,y,z))
                    f.write("{} {} {}\n".format(x,y,z))
                    f.flush() # Ensure that during each sample, the cloud can be visualized
                data = []
                x = deltaX + x
            s.close()
            f.close()
    except:
        print("Excption Occured")
        print("Closing: " + s.name)
        s.close()