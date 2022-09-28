import matplotlib.pyplot as plt

def read_sh_hv(lines):
    shhv = []
    for item in lines:
        data = item.strip().split(" ")
        crate = data[0]
        slot = int(data[1][1:])
        chan = data[3:]

        if crate == "rpi17:2001":
            if slot == 2:
                shhv = chan[3:]
            elif slot > 2 and slot < 9:
                shhv.extend(chan)
            else:
                shhv.extend(chan[:3])

        if crate == "rpi18:2001":
            if slot > 4 and slot < 13:
                shhv.extend(chan)
            elif slot == 13:
                shhv.extend(chan[:9])

    return shhv

with open('hv_set/hv_calibrated_run_11845_15mV_11_18_2021.set', 'r') as f:
    lines = f.readlines()

shhv = read_sh_hv(lines)
shhv = [eval(i) for i in shhv]

#print(shhv)

plt.plot(shhv, 'o')
plt.show()
