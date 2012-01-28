import zero
def __init__():
	pass

def step(count = 1):
	thread = zero.debugger().current_thread()
	thread.step(zero.Step.OverStatement, count)

def msg(txt):
	zero.debugger().message(txt, zero.Message.Info)

