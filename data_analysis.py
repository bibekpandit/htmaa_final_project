import matplotlib.pyplot as plt
import numpy as np



def make_plot(one_chunk, name):
    print(len(one_chunk))
    t = np.arange(len(one_chunk))
    x = [line[0] for line in one_chunk]
    y = [line[1] for line in one_chunk]
    z = [line[2] for line in one_chunk]
 
    plt.plot(t, x)
    plt.plot(t, y)
    plt.plot(t, z)
    plt.title(name)
    plt.show()

def make_accel_plot(one_chunk, name):
    print(len(one_chunk))
    t = np.arange(len(one_chunk))
    x = [line[0] for line in one_chunk]
    y = [line[1] for line in one_chunk]
    z = [line[2] for line in one_chunk]

    w = [x[i]*x[i] + y[i]*y[i] + z[i]*z[i] for i in range(len(x))]

    # plt.plot(t, x)
    # plt.plot(t, y)
    # plt.plot(t, z)
    plt.plot(t,w)
    plt.title(name)
    plt.show()


file = 'data.txt'

with open(file) as f:
    lines = f.readlines()

lines = [x.strip() for x in lines]

all_data = []

for line in lines:
    info = line.split(" ")
    info[1] = float(info[1])
    info[2] = float(info[2])
    info[3] = float(info[3])
    all_data.append(info)

state_data = {"walk":[], "still":[], "fall":[], "lie":[]}

one_chunk = []
current_state = all_data[0][0]
# print(current_state)
for line in all_data[1:]:
    # print(line)
    if line[1] == 0 and line[2] == 0 and line[3] == 0:
        state_data[current_state].append(one_chunk)
        current_state = line[0]
        one_chunk = []
    else:
        one_chunk.append([line[1], line[2], line[3]])
state_data[current_state].append(one_chunk)
# print(state_data)

def get_var(x):
    step_size = 20
    x_arrays = [x[i:i+step_size] for i in range(0, len(x), step_size) if i+step_size < len(x)]
    num_arrays = [np.array(x_sub) for x_sub in x_arrays]
    return [round(np.var(arr),2) for arr in num_arrays]

def get_mean(x):
    x_arrays = [x[i:i+10] for i in range(0, len(x), 10) if i+10 < len(x)]
    num_arrays = [np.array(x_sub) for x_sub in x_arrays]
    return [round(np.mean(arr),2) for arr in num_arrays[:20]]

print("FALL")
i = 1
for chunk in state_data["fall"]:
    print(len(chunk))
    # make_plot(chunk, "still_" + str(i))
    # make_accel_plot(chunk, "still_" + str(i))
    x = [line[0] for line in chunk]
    y = [line[1] for line in chunk]
    z = [line[2] for line in chunk]

    # print("x mean:", get_mean(x))
    print("x var:", get_var(x))
    # print("y mean:", get_mean(y))
    print("y var:", get_var(y))
    # print("z mean:", get_mean(z))
    print("z var:", get_var(z))

    print("x total var:", np.var(np.array(x)))
    print("y total var:", np.var(np.array(y)))
    print("z total var:", np.var(np.array(z)))
    

    i += 1
    break
# make_accel_plot(state_data["still"][0], "still")
# make_accel_plot(state_data["still"][0], "still")
# make_plot(state_data["lie"][0], "lie")