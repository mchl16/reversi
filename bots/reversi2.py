from copy import deepcopy as cp
import sys

DEPTH=5
DIFF=DEPTH

debug=len(sys.argv)>=2 and sys.argv[1]=='debug'


class reversi_array:
    def __init__(self):
        self.storage=[0,0]

    def __getitem__(self,i):
        a,b=i//32,(i&31)<<1
        return (self.storage[a]>>b)&3

    def __setitem__(self,i,val):
        b,a=(i&31)<<1,i//32
        kaku=3<<b
        self.storage[a]&=~kaku
        val<<=b
        self.storage[a]|=val

    # a,b=1,0
    # for i in range(4):
    #     yield (a,b)
    #     a,b=-b,a

    # a,b=1,1
    # for i in range(4):
    #     yield (a,b)
    #     a,b=-b,a

class reversi:
    def __init__(self):
        self.color=1
        self.board=bytearray(64) #reversi_array() 
        self.board[27]=2
        self.board[28]=1
        self.board[35]=1
        self.board[36]=2
        self.rank=0

    def place_piece(self,field):
        self.board[field]=self.color
        self.rank+=(1 if self.color==1 else -1)*reversi.f(field)
        f1,f2=field//8,field%8
        for x,y in ((1,0),(0,1),(-1,0),(0,-1),(1,1),(-1,1),(-1,-1),(1,-1)):
            g1,g2=f1+x,f2+y
            while 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==3-self.color:
                g1,g2=g1+x,g2+y
            
            if 0>g1 or 8<=g1 or 0>g2 or 8<=g2 or self.board[8*g1+g2]!=self.color:
                continue
            g1,g2=f1+x,f2+y
            while 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==3-self.color:
                self.board[8*g1+g2]=self.color
                self.rank+=(2 if self.color==1 else -2)*reversi.f(8*g1+g2)
                g1,g2=g1+x,g2+y

    def check_move(self,field):
        if field<0: 
            return True
        f1,f2=field//8,field%8
        if self.board[field]!=0: return False
        for x,y in ((1,0),(0,1),(-1,0),(0,-1),(1,1),(-1,1),(-1,-1),(1,-1)):
            g1,g2=f1+x,f2+y
            while 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==3-self.color:
                g1,g2=g1+x,g2+y
            
            if 0<=g1<8 and 0<=g2<8 and self.board[8*g1+g2]==self.color:
                if g1!=f1+x or g2!=f2+y: return True

    def move(self,field):
        # if debug and not self.check_move(field):
        #     raise Exception("Illegal move")
        
        if field>=0: self.place_piece(field)
        self.color=3-self.color
        self.rank=self.rankp()   

    def print_board(self):
        for i in range(64):
            print('.BW'[self.board[i]],end='\n' if i&7==7 else '')
        print()

    def f(x):
        # return (100,-20,10,5,5,10,-20,100,\
        #         -20,-50,-2,-2,-2,-2,-50,-20,\
        #         10,-2,-1,-1,-1,-1,-2,10,\
        #         5,-2,-1,-1,-1,-1,-2,5,\
        #         5,-2,-1,-1,-1,-1,-2,5,\
        #         10,-2,-1,-1,-1,-1,-2,10,\
        #         -20,-50,-2,-2,-2,-2,-50,-20,\
        #         100,-20,10,5,5,10,-20,100)[x]
        
        return (4,-3,2,2,2,2,-3,4,\
                -3,-4,-1,-1,-1,-1,-4,-3,\
                2,-1,1,0,0,1,-1,2,\
                2,-1,0,1,1,0,-1,2,\
                2,-1,0,1,1,0,-1,2,\
                2,-1,1,0,0,1,-1,2,\
                -3,-4,-1,-1,-1,-1,-4,-3,\
                4,-3,2,2,2,2,-3,4)[x]

    def rankp(self): #temporary
        res1,res2,res3,res4,res5=0,0,0,0,0
        for i in range(64):
            if self.board[i]==1: 
                res1+=1
                res5+=reversi.f(i)
            elif self.board[i]==2: 
                res2+=1
                res5-=reversi.f(i)
            if self.check_move(i): res3+=1

        self.color=3-self.color
        for i in range(64):
            if self.check_move(i): res4+=1
        self.color=3-self.color
        
        R=res3+res4
        if R==0:
            if res1<res2: return -1e18
            elif res1>res2: return 1e18
            return 0
        return 100*((res1-res2)/(res1+res2)+5*(res3-res4)/R)+res5

    def play(self):
        while True:
            self.print_board()
            dirty=False
            for i in range(64):
                if self.check_move(i):
                    dirty=True
                    break
            if not dirty: self.color=3-self.color

            s=list(map(int,input().split()))
            try:
                self.move(8*s[0]+s[1])
            except Exception as e:
                print(e)

    def hash(self):
        T=reversi_array()
        for i in range(64): T[i]=self.board[i]
        return (T.storage[0],T.storage(1),self.color)

dict={}

def alphabeta(pos,a=-1e18,b=1e18,depth=DEPTH,last=-1,passed=False):

    if DEPTH-depth>=DIFF: return alphabeta2(pos,a,b,depth,last,passed)
    black=(pos.color==1)
    val=pos.rank
    res=last
    if depth==0:
        # print(depth,a,b,res,file=sys.stderr)
        return (pos.rank,res)

    t=None
    t=list(filter(lambda i: pos.check_move(i),range(64)))
    if len(t)==0:
        if passed:
            cnt=0
            for i in range(64):
                cnt+=(0,1,-1)[pos.board[i]]
            if cnt!=0:
                if cnt>0: cnt=1
                else: cnt=-1
            return (1e18*cnt,-1)

        pos.move(-1)
        return alphabeta2(pos,a,b,depth-1,-1,True)
    T=[None for _ in range(64)]
    for j in t:
        T[j]=cp(pos)
        T[j].move(j)
    t.sort(lambda x: (-1 if black else 1)*T[x].rank)

    for i in t:
        pos2=T[i]
        ab=alphabeta(pos2,a,b,depth-1,i,False)
    #  print(depth,black,ab[0],ab[1],file=sys.stderr)
        if black:
            if ab[0]>a:
                a=ab[0]
                val,res=a,i
        else:
            if ab[1]<b:
                b=ab[0]
                val,res=b,i
        if a>b: 
            break
    # print(depth,a,b,res,file=sys.stderr)
    return (val,res)

def alphabeta2(pos,a,b,depth,last,passed):
    black=(pos.color==1)
    val=pos.rank
    res=last
    if depth==0:
        # print(depth,a,b,res,file=sys.stderr)
        return (val,res)
    
    dirty=False
    for i in range(64):
        if not pos.check_move(i): continue
        dirty=True
        pos2=cp(pos)
        pos2.move(i)
        ab=alphabeta2(pos2,a,b,depth-1,i,False)
    #  print(depth,black,ab[0],ab[1],file=sys.stderr)
        if black:
            if ab[0]>a:
                a=ab[0]
                val,res=a,i
        else:
            if ab[1]<b:
                b=ab[0]
                val,res=b,i
        if a>b: 
            break
    # print(depth,a,b,res,file=sys.stderr)
    if dirty: return (val,res)

    if passed:
        cnt=0
        for i in range(64):
            cnt+=(0,1,-1)[pos.board[i]]
        if cnt!=0:
            if cnt>0: cnt=1
            else: cnt=-1
        return (1e18*cnt,-1)

    pos.move(-1)
    ab=alphabeta2(pos,a,b,depth-1,-1,True)
    if black:
        if ab[0]>a:
            a=ab[0]
            val,res=a,-1
    else:
        if ab[1]<b:
            b=ab[0]
            val,res=b,-1
    return (val,res)

g=reversi()
print("RDY")

if debug:
    g.play()
else:
    while True:
        xd=input().split()
        cmd,args=xd[0],list(map(int,xd[3::]))

        if cmd=="HEDID":
            g.move(8*args[1]+args[0])
            # g.print_board()
            x=alphabeta(g)[1]
            print("IDO",-1 if x<0 else x%8,x//8)
            g.move(x)
            # g.print_board()
        elif cmd=="ONEMORE":
            g=reversi()
            print("RDY")
        elif cmd=="BYE":
            exit(0)
        elif cmd=="UGO":
            x=alphabeta(g)[1]
            print("IDO",-1 if x<0 else x%8,x//8)
            g.move(x)
            # g.print_board()