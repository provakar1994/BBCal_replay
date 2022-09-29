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

with open('hv_set/hv_updated_sh_ps_20mV_10_11_2021.set', 'r') as f:
    lines0 = f.readlines()

with open('hv_set/hv_calibrated_run_13137_20mV_1_10_2022.set', 'r') as f:
    lines1 = f.readlines()

with open('hv_set/hv_calibrated_run_1462_20mV_09_18_2022.set', 'r') as f:
    lines2 = f.readlines()

shhv0 = read_sh_hv(lines0)
shhv0 = [eval(i) for i in shhv0]

shhv1 = read_sh_hv(lines1)
shhv1 = [eval(i) for i in shhv1]

shhv2 = read_sh_hv(lines2)
shhv2 = [eval(i) for i in shhv2]

#print(shhv)

plt.plot(shhv0, 'o', color='green', label='Oct 11, 2021')
plt.plot(shhv1, 'o', color='blue', label='Jan 10, 2022')
plt.plot(shhv2, 'o', color='red', label='Sep 18, 2022')

plt.title('Shower HV Overlay [20mV Trigger Amplitude]')
plt.xlabel('Shower Block')
plt.ylabel('HV (V)')
plt.legend()
plt.grid()
plt.show()
