import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import time

from matplotlib.animation import FuncAnimation, PillowWriter

df = pd.DataFrame(pd.read_csv("logs/obd2_log20250603_185233.csv"))
# print(df.head())

fig,ax1 = plt.subplots()
# ax1.set_xlim(0, df.shape[0])
# ax2=ax1.twinx()


# fig = df.plot(y="0xc", kind="line", label="0xc", xlim=(0, df.shape[0]), ylim=(500, 10000))
# df["0xc"].plot(title="data")
# df["0xc"].ewm(span=5).mean().plot(label="10-period EWMA")
# plt.show()
df["row"]=np.arange(df.shape[0])
# df = df.apply(pd.to_numeric, errors='ignore')

# ax1.plot(df["row"], df["0xc"])
# ax2=ax1.twinx()
# ax2.plot(df["row"], df["0x11"], color="orange")
# ax1.set_title("0xc Data Over Time")
# plt.show()
# exit()
def animate(i):
    plt.clf()
    # ax2.clear() 
    # print(i)
    df_subset = df[df["row"] <= i]
    # ax1.set_xlabel("Time")
    # ax1.set_xticks(np.arange(0, df.shape[0], step=100))
    ax2=ax1.twinx()
    ax3=ax2.twinx()
    # print(df_subset)
    # fig.add_subplot(111)

    # df_subset["0xc"].plot(ax=ax1, kind="line", label="0xc", color="blue")


    timestamp= df_subset.iloc[i]["Timestamp"]
    df_subset["0x11"].plot(ax=ax2,y="0x11", kind="line", label="Throttle Pos",xlim=(0, df.shape[0]), ylim=(0, 100), color="orange")
    df_subset["0xc"].plot(ax=ax3,y="0x11", kind="line", label="RPM",xlim=(0, df.shape[0]), ylim=(0, 10000), color="blue")
    df_subset["0x4"].plot(ax=ax2,y="0x4", kind="line", label="engine load",xlim=(0, df.shape[0]), ylim=(0, 100), color="green")
    ax2.legend(loc='upper left')
    ax3.legend(loc='upper right')
    # ax1.plot(df_subset["row"], df_subset["0xf"],color="blue", label="0xc")
    # ax2.plot(df_subset["row"], df_subset["0x11"], color="orange")
    # ax1.set_title("0xc Data Over Time")

    # ax1.plot(df_subset["row"], df_subset["0xc"], label="0xc", color="blue",kind="line",xlim=(0, df.shape[0]), ylim=(500, 10000), title=timestamp)
    
    # speed=plt.twinx().plot(y=df_subset["0x11"], kind="line", label="0x11", xlim=(0, df.shape[0]), ylim=(0, 100), title=timestamp, color="orange")
    # print(timestamp)
    # df_subset[["0xc","0x11","0xb"]].plot(subplots=True,layout=(3,1))
    
    # df_subset["0x11"].twinx().plot(y="0x11", kind="line", label="0x11",xlim=(0, df.shape[0]), ylim=(0, 100),title=timestamp)
    # ax1.plot(df_subset["row"], df_subset["0xc"], label="0xc", color="blue")
    # plt.legend()
    # plt.figtext(0,0,df_subset.iloc[i]["0x1f"])
    plt.title(timestamp)
    # plt.xlabel("Row")
animation = FuncAnimation(fig, animate, frames=df.shape[0], interval=10)

plt.show()