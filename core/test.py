import tkinter
import threading as th
import time

win = tkinter.Tk()
win.title("system")
'''
供用户通过拖拽指示器改变变量的值，可以水平，也可以垂直
orient       设置指示器方向（水平，垂直）
tkinter.HORIZONTAL   水平
tkinter.VERTICAL     垂直
length               水平时表示宽度，垂直时表示高度
tickinterval          选择值将会为该值的倍数
'''
#默认垂直，由上向下
scale=tkinter.Scale(win,from_=0,to=100,orient=tkinter.HORIZONTAL,
                    tickinterval=100,length=200)
scale.pack()
#设置初始值
scale.set(20)

def get_scale_pos():
    while True:
        print(scale.get())
        time.sleep(1/50)

a = th.Thread(target=get_scale_pos)
a.start()

def showNum():
    print(scale.get())

tkinter.Button(win,text="打印",command=showNum).pack()

win.mainloop()