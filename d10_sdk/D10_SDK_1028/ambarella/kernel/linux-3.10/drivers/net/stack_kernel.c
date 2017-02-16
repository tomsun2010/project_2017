/** 
 *  @file stack_kernel.c
 *  @brief
 *  @version 0.0.1
 *  @since 0.0.1
 *  @author 
 *  @date 
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/bcd.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/ptrace.h>
#include <linux/netlink.h>
#include <linux/socket.h> 
#include <linux/skbuff.h> 
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <net/sock.h>
#include <asm/ptrace.h>
#include <linux/sched.h>


#define AL_STACK_K_PID        78
#define NL_STACK              27            
#define PROGRAM_NAME         (56)
#define STACK_BUFFER_SIZE   (1024)
//#ifdef AMBA_SDK_XIAOYI_ALTER
static int stack_kernel_num = 0;
//#endif

MODULE_LICENSE("Dual BSD/GPL");


DEFINE_SEMAPHORE(receive_sem);

/**
 * �ں�netlink���û��ռ䷢����Ϣ�Ŀ��ƿ���Ϣ
 */
struct msg_to_usr
{
    struct nlmsghdr hdr;
};

/**
 * �ں�netlink���û��ռ䷢�͵���Ϣ
 */
struct u_stack_packet
{
    int flags;                          //0x17:֪ͨ�û�����maps��0x27:֪ͨ�û�����ջ����
    int packet_num;                     
    int packet_max;                     
    unsigned long mem_access_addr;      
    unsigned int  access_write;         
    unsigned long stack_start;          
    unsigned long stack_end;            
    int stack_length;                   
    pid_t pid;                        
    struct pt_regs regs;              
    unsigned char name[PROGRAM_NAME];  
    unsigned char stack_buffer[STACK_BUFFER_SIZE]; 
};

/**
 * ������¼�û��ռ䷢����Ϣ�Ľ���
 */
struct usr_proc
{
    int usr_pid;
    rwlock_t lock;
};
struct usr_proc user_proc;

static struct sock *stack_sock; 


/**
 * @fn send_to_user
 * @brief �ں�netlink������Ϣ���û��ռ�
 * @param[in] info:���͵�����
 * @return 
 * @retval 
 */
int send_to_user(struct u_stack_packet *info)
{
    int size;
    int ret_val;
    unsigned char *old_tail;
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    struct u_stack_packet *packet;

    size = NLMSG_SPACE(sizeof(*info));

    skb = alloc_skb(size, GFP_ATOMIC);
    old_tail = skb->tail;

    nlh = (struct nlmsghdr *)nlmsg_put(skb, 0, 0, AL_STACK_K_PID, size-sizeof(*nlh), 0);

    packet = NLMSG_DATA(nlh);
    memset(packet, 0, sizeof(*info));
    memcpy(packet, info, sizeof(struct u_stack_packet));

    nlh->nlmsg_len = skb->tail - old_tail;
    printk("nlh->nlmsg_len = 0x%x\n",nlh->nlmsg_len);

    NETLINK_CB(skb).dst_group = 0;

    read_lock_bh(&user_proc.lock);
    ret_val = netlink_unicast(stack_sock, skb, user_proc.usr_pid, MSG_WAITALL);
    read_unlock_bh(&user_proc.lock);
    mdelay(20);
	
    stack_kernel_num++;
	
    return 0;
}

/**
 * @fn print_reg
 * @brief ��ӡ��stack��Ϣ
 * @param[in] tsk    ����ṹ��һ������(�߳�)��Ӧһ��task�ṹ
 * @param[in] regs   �Ĵ���
 * @param[in] addr   
 * @param[in] fsr 
 * @return 
 * @retval 
 *
 * ����:
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * ++  ipc is dead (PID: 887)
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 
 * SP = 0xbd1ffd18  FP = 0xbd1ffd6c  IP = 0x4003d2a0  LR = 0x4002e0dc  PC = 0x0015f7ec
 * CPSR = 0x60000010
 * access: [Write], addr = 0x00000000 
 *
 * r0 = 0x00000000  r1 = 0xffffffff  r2 = 0x0000dead  
 * r3 = 0x00000000  r4 = 0xbd1ffe20  r5 = 0x4003d0e8  
 * r6 = 0x00000000  r7 = 0x0000000a  r8 = 0x00000000  
 * r9 = 0x00000000  r10 = 0x40041360  r11 = 0xbd1ffd6c  
 * r12 = 0x4003d2a0  r13 = 0xbd1ffd18  r14 = 0x4002e0dc  
 * r15 = 0x0015f7ec  r16 = 0x60000010  r17 = 0xffffffff  
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
void print_reg(struct task_struct *tsk, struct pt_regs *regs, unsigned long addr, unsigned int fsr)
{
    int i = 0, j = 0;
    int reg_num = 0;
    
    printk("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printk("++  %s is dead (PID: %d)\n", tsk->comm, tsk->pid);
    printk("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printk("\n");
    printk("SP = 0x%08x  FP = 0x%08x  IP = 0x%08x  LR = 0x%08x  PC = 0x%08x\n", 
        (unsigned int)regs->ARM_sp, (unsigned int)regs->ARM_fp, (unsigned int)regs->ARM_ip, (unsigned int)regs->ARM_lr, (unsigned int)regs->ARM_pc);
    printk("CPSR = 0x%08x\n", (unsigned int)regs->ARM_cpsr);
    
    if (fsr & (1 << 11))    /* write */
        printk("access: [Write], addr = 0x%08x \n", (unsigned int)addr);
    else
        printk("access: [Read], addr = 0x%08x \n", (unsigned int)addr);

    printk("\n");

    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 3; j++)
        {
            reg_num = i * 3 + j;
            printk("r%d = 0x%08x  ", reg_num, (unsigned int)regs->uregs[reg_num]);
        }
        printk("\n");
    }
    
    printk("\n");
    printk("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printk("\n\n");
}

/**
 * @fn my_show_stack
 * @brief �Զ���ļ�¼stack��Ϣ�ĺ���
 * @param[in] tsk    ����ṹ��һ������(�߳�)��Ӧһ��task�ṹ
 * @param[in] sp     ջָ��,ջ��
 * @param[in] regs   �Ĵ���
 * @param[in] addr   ����ʱ���ʵ��ڴ��ַ
 * @param[in] fsr    ����ʱ���ʵ��ڴ��ַ��д���Ƕ�
 * @return 
 * @retval 
 */
void my_show_stack(struct task_struct *tsk, unsigned long *sp, struct pt_regs *regs, unsigned long addr, unsigned int fsr)
{
    int i = 0;
    int stack_len = 0;
    int upload_times = 0, remain_bytes = 0;
    unsigned long sp_addr = 0;
    struct vm_area_struct *vma = NULL;
    struct u_stack_packet info;

    stack_kernel_num++;

    if ( NULL == tsk || NULL == sp || NULL == regs)
    {
        return;
    }

    print_reg(tsk, regs, addr, fsr);

    memset(&info, 0, sizeof(info));

    sp_addr = (unsigned long)sp;
    info.stack_start= sp_addr;

    info.mem_access_addr = addr;

    if (fsr & (1 << 11))
        info.access_write = 1;
    else
        info.access_write = 0;
    
    vma = find_vma(tsk->mm, sp_addr);
    if (NULL == vma)
        return;

    if ((sp_addr < vma->vm_start) || (sp_addr > vma->vm_end))
    {
        printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printk("kernel: The stack pointer is not in vma\n");
        printk("kernel: start(0x%#08x) < SP(0x%#08x) < end(0x%#08x)\n",
            (unsigned int)vma->vm_start, (unsigned int)sp_addr, (unsigned int)vma->vm_end);
        printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");
        return;
    }   
    else
    {
        printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        printk("kernel: vm_start(0x%#08x) < sp(0x%#08x) < vm_end(0x%#08x)\n",
            (unsigned int)vma->vm_start, (unsigned int)sp_addr, (unsigned int)vma->vm_end);
        printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");
    }
    
    info.stack_end = vma->vm_end;

    memcpy(&info.regs, regs, sizeof(*regs));

    stack_len = vma->vm_end- regs->ARM_sp;
    info.stack_length = stack_len;

    info.pid = tsk->pid;

    strncpy(info.name, tsk->comm, sizeof(info.name));
    
    info.flags = 0x17;  
    send_to_user(&info);
    ssleep(1);

    info.flags = 0x27;          

    if (stack_len > STACK_BUFFER_SIZE)
    {
        upload_times= stack_len / STACK_BUFFER_SIZE;
        remain_bytes = stack_len % STACK_BUFFER_SIZE;

        info.packet_max = upload_times + 1;

        for (i = 0; i < upload_times; i++)
        {
            info.packet_num = i + 1;
            memset(info.stack_buffer, 0, sizeof(info.stack_buffer));
            memcpy(info.stack_buffer, (void *)(sp_addr + STACK_BUFFER_SIZE * i), STACK_BUFFER_SIZE);   
            send_to_user(&info);
        }

        info.packet_num = upload_times + 1;
        memset(info.stack_buffer, 0, sizeof(info.stack_buffer));
        memcpy(info.stack_buffer, (void *)(sp_addr + STACK_BUFFER_SIZE * upload_times), remain_bytes);
        send_to_user(&info);
    }
    else    
    {
        info.packet_max= 1;
        info.packet_num = 1;
        memset(info.stack_buffer, 0, sizeof(info.stack_buffer));
        memcpy(info.stack_buffer, (void *)(sp_addr), (stack_len));
        send_to_user(&info);
    }
}

/**
 * @fn kernel_receive
 * @brief �ں�sock���յ��û��ռ���Ϣ��Ĵ���������Ҫ���Ǽ�¼�û��ռ䷢����Ϣ�Ľ���id
 * @param[in] 
 * @return 
 * @retval 
 */
void kernel_receive(struct sk_buff *__skb)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb;

    /* ����SMP,��д�������ⲻͬCPU�����ٽ��������� */
    if(down_trylock(&receive_sem))
        return;

    /* �������ü���,��������skb */
    skb = skb_get(__skb);

    if (skb->len >= NLMSG_SPACE(0)) 
    {
        nlh = nlmsg_hdr(skb);       
        write_lock_bh(&user_proc.lock);
        user_proc.usr_pid = NETLINK_CREDS(skb)->pid;
        write_unlock_bh(&user_proc.lock);
        printk("stack_sock->nlmsg_type = %d stack_sock->nlmsg_pid = %d\n", nlh->nlmsg_type, nlh->nlmsg_pid);
    }

    up(&receive_sem);
}

/**
 * @fn stack_socket_init
 * @brief ģ���ʼ������Ҫ�Ǵ����ں�netlink
 * @param[in] 
 * @return 
 * @retval 
 */
int stack_socket_init(void)
{
    struct netlink_kernel_cfg cfg = {
		.input	= kernel_receive,
	};

    rwlock_init(&user_proc.lock);
    
    stack_sock = netlink_kernel_create(&init_net, NL_STACK, &cfg);
    if(!stack_sock)
    {
        printk("create netlink_kernel erro\n");
        return -1;
    }
    
    return 0;
}

/**
 * @fn stack_socket_exit
 * @brief ģ���˳�����Ҫ�ǽ��ں�netlink�ͷ�
 * @param[in] 
 * @return 
 * @retval 
 */
void stack_socket_exit(void)
{
    printk("stack_socket exit\n");
    sock_release(stack_sock->sk_socket);
}

module_init(stack_socket_init);
module_exit(stack_socket_exit);


