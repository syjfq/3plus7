from CommunicationTool import *

# 生成控制指令（参考案例）
jidong_time = [500, 2500, 3000, 4000, 4500, 5500]


def create_action_cmd(info, step_num):
    # if (step_num <= jidong_time[0]):
    if (step_num - 1) % 800 <200:
        output_cmd = SendData()
        output_cmd.sPlaneControl.CmdIndex = 1 # 动作序号
        output_cmd.sPlaneControl.CmdID = 1 #动作编号
        if (step_num == jidong_time[0]):
            output_cmd.sPlaneControl.isApplyNow = False
        output_cmd.sPlaneControl.isApplyNow = True
        output_cmd.sPlaneControl.CmdHeadingDeg = 90
        output_cmd.sPlaneControl.CmdAlt = 7000  #10000
        output_cmd.sPlaneControl.CmdSpd = 0.9
        output_cmd.sPlaneControl.TurnDirection = 1
    elif 200<=(step_num - 1) % 800 <400:
        output_cmd = SendData()
        output_cmd.sPlaneControl.CmdIndex = 1 # 动作序号
        output_cmd.sPlaneControl.CmdID = 1 #动作编号
        if (step_num == jidong_time[0]):
            output_cmd.sPlaneControl.isApplyNow = False
        output_cmd.sPlaneControl.isApplyNow = True
        output_cmd.sPlaneControl.CmdHeadingDeg = 0
        output_cmd.sPlaneControl.CmdAlt = 7000  #10000
        output_cmd.sPlaneControl.CmdSpd = 0.9
        output_cmd.sPlaneControl.TurnDirection = 1
    elif 400<=(step_num - 1) % 800 <600:
        output_cmd = SendData()
        output_cmd.sPlaneControl.CmdIndex = 1 # 动作序号
        output_cmd.sPlaneControl.CmdID = 1 #动作编号
        if (step_num == jidong_time[0]):
            output_cmd.sPlaneControl.isApplyNow = False
        output_cmd.sPlaneControl.isApplyNow = True
        output_cmd.sPlaneControl.CmdHeadingDeg =-90
        output_cmd.sPlaneControl.CmdAlt = 7000  #10000
        output_cmd.sPlaneControl.CmdSpd = 0.9
        output_cmd.sPlaneControl.TurnDirection = 1
    # elif(step_num<=jidong_time[1]):
    #     output_cmd = SendData()
    #     output_cmd.sPlaneControl.CmdIndex = 2
    #     output_cmd.sPlaneControl.CmdID = 6
    #     if(step_num==jidong_time[1]):
    #         output_cmd.sPlaneControl.isApplyNow = False
    #     output_cmd.sPlaneControl.isApplyNow = True
    #     output_cmd.sPlaneControl.CmdHeadingDeg = 180
    #     output_cmd.sPlaneControl.CmdAlt = 10000
    #     output_cmd.sPlaneControl.CmdSpd = 0.9
    #     output_cmd.sPlaneControl.CmdNy = 2
    #     output_cmd.sPlaneControl.CmdThrust = 120
    #     output_cmd.sPlaneControl.TurnDirection = -1
    #
    # elif(step_num<=jidong_time[2]):
    #     output_cmd = SendData()
    #     output_cmd.sPlaneControl.CmdIndex = 3
    #     output_cmd.sPlaneControl.CmdID = 1
    #     output_cmd.sPlaneControl.VelType = 0
    #     output_cmd.sPlaneControl.CmdSpd = 0.9
    #     if (step_num == jidong_time[2]):
    #         output_cmd.sPlaneControl.isApplyNow = False
    #     output_cmd.sPlaneControl.isApplyNow = True
    #     output_cmd.sPlaneControl.CmdHeadingDeg = 180
    #     output_cmd.sPlaneControl.CmdAlt = 10000
    #
    # elif (step_num <= jidong_time[3]):
    #     output_cmd = SendData()
    #     output_cmd.sPlaneControl.CmdIndex = 4
    #     output_cmd.sPlaneControl.CmdID = 7
    #     output_cmd.sPlaneControl.VelType = 0
    #     output_cmd.sPlaneControl.CmdSpd = 0.8
    #     if(step_num==jidong_time[3]):
    #         output_cmd.sPlaneControl.isApplyNow = False
    #     output_cmd.sPlaneControl.isApplyNow = True
    #     output_cmd.sPlaneControl.CmdPitchDeg = -20
    #     output_cmd.sPlaneControl.CmdHeadingDeg = 180
    #     output_cmd.sPlaneControl.CmdAlt = 4000
    #
    # elif(step_num <= jidong_time[4]):
    #     output_cmd = SendData()
    #     output_cmd.sPlaneControl.CmdIndex = 5
    #     output_cmd.sPlaneControl.CmdID = 1
    #     output_cmd.sPlaneControl.VelType = 0
    #     output_cmd.sPlaneControl.CmdSpd = 0.9
    #     if (step_num == jidong_time[4]):
    #         output_cmd.sPlaneControl.isApplyNow = False
    #     output_cmd.sPlaneControl.isApplyNow = True
    #     output_cmd.sPlaneControl.CmdHeadingDeg = 180
    #     output_cmd.sPlaneControl.CmdAlt = 5000
    #
    # elif(step_num <= jidong_time[5]):
    #     output_cmd = SendData()
    #     output_cmd.sPlaneControl.CmdIndex = 6
    #     output_cmd.sPlaneControl.CmdID = 3
    #     output_cmd.sPlaneControl.VelType = 0
    #     output_cmd.sPlaneControl.CmdSpd = 0.9
    #     if (step_num == jidong_time[5]):
    #         output_cmd.sPlaneControl.isApplyNow = False
    #     output_cmd.sPlaneControl.isApplyNow = True
    #     output_cmd.sPlaneControl.CmdHeadingDeg = 180

    else:
        output_cmd = SendData()
        output_cmd.sPlaneControl.CmdIndex = 7
        output_cmd.sPlaneControl.CmdID = 1
        output_cmd.sPlaneControl.VelType = 0
        output_cmd.sPlaneControl.CmdSpd = 0.9
        output_cmd.sPlaneControl.isApplyNow = True
        output_cmd.sPlaneControl.CmdHeadingDeg =0  #180
        output_cmd.sPlaneControl.CmdAlt = 7000  #5000

    return output_cmd


# 规整上升沿
def check_cmd(cmd, last_cmd):
    # if last_cmd is None:
    #     cmd.sPlaneControl.isApplyNow = False
    #     cmd.sOtherControl.isLaunch = 0
    #     cmd.sSOCtrl.isNTSAssigned = 0
    # else:
    #     if cmd.sPlaneControl == last_cmd.sPlaneControl:
    #         cmd.sPlaneControl.isApplyNow = False
    #     if cmd.sSOCtrl == last_cmd.sSOCtrl:
    #         cmd.sSOCtrl.isNTSAssigned = 0
    return cmd


# 获取传输数据，生成对应无人机command指令，并传输指令逻辑
def solve(platform, plane):
    global save_last_cmd
    if plane==2:
        if platform.step > save_last_cmd[plane][1]:
            # if platform.recv_info.AlarmList[0].MisAzi != 0:
            #     print(platform.recv_info.DroneID, ": vars(AlarmList[0])", vars(platform.recv_info.AlarmList[0]))
            cmd_created = create_action_cmd(platform.recv_info, platform.step)  # 生成控制指令
            # 保存上一个发送的指令
            save_last_cmd[plane][0] = cmd_created  # 更新保存指令

            cmd_created = check_cmd(cmd_created, save_last_cmd[plane][0])  # 比较得到上升沿
            platform.cmd_struct_queue.put(cmd_created)  # 发送数据
            save_last_cmd[plane][1] = save_last_cmd[plane][1] + 1


def main(IP, Port, drone_num):
    data_serv = DataService(IP, Port, drone_num)  # 本机IP与设置的端口，使用config文件
    data_serv.run()  # 启动仿真环境

    global save_last_cmd  # 用于比较指令变化的字典全局变量
    save_last_cmd = {}

    for plane in data_serv.platforms:  # 初始化全局变量为None
        save_last_cmd[plane] = [None, 0]

    while True:  # 交互循环
        try:
            for plane in data_serv.platforms:
                # print(plane, ": ", data_serv.platforms[plane].recv_info)
                solve(data_serv.platforms[plane], plane)  # 处理信息
                print(plane, "'s step is  ", data_serv.platforms[plane].step)
        except Exception as e:
            print("Error break", e)
            break
    data_serv.close()


if __name__ == "__main__":
    IP = "192.168.0.100"#"192.168.66.90"
    Port = 60003
    drone_num = 4
    main(IP, Port, drone_num)
