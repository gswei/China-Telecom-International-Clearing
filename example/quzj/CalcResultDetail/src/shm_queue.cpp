#include "shm_queue.h"

using namespace tpss;

#define MY_TEMPLATE template<typename T_DATA,int PI_N,typename T_USER_HEAD,int VER,typename T_HANDLE >

#define QUEUE  export ShmQueue<T_DATA, PI_N, T_USER_HEAD, VER, T_HANDLE>

MY_TEMPLATE
char const* QUEUE::GetName()const
{
    return  array_.GetName();
}

MY_TEMPLATE
bool QUEUE::CreateShm()
{
    SetCreateShm( 0 );
}

MY_TEMPLATE
bool QUEUE::Attach( bool is_read_only )
{
    return SetAttachToShm();
}

MY_TEMPLATE
bool QUEUE::Detach()
{
    return SetDetachFromShm();
}

//获取队列头部的元素
MY_TEMPLATE
T_DATA* QUEUE::Top()
{
    if( IsEmpty ) {
        return NULL;
    }

    return &array_.Get( head_->head_node );
}

//向队列尾部压入一个数据
MY_TEMPLATE
bool QUEUE::Push( const T_DATA& elem )
{
    ShmQueueNode node( elem );

    if( -1 == head_->free_head_node ) {
        //回收队列为空，说明[0, size)单元数组全部装有有效数据， 只需把当前元素插入到size位置就行

        HANDLE h( size );
        ret = array_.Add( node, h );

        if( ret ) {
            if( size ) {
                array_.Get( head_->tail_node ).next = size;
                head_->tail_node = size;
            } else {
                head_->tail_node = size;
                head_->tail_node = size;
            }
        }

        size ++;
        return Add( node, h );
    } else {
        //回收栈不为空，将现有数据加入到回收队列的队首位置
        node = array_.Get( head_->free_head_node );	//获取回收栈的数组单元
        ns_shm_data::T_SHM_SIZE tmp = head_->free_head_node;
        head_->free_head_node = node->next;			//回收栈弹出一个数组单元

        if( size ) {									//队列为非空
            array_.Get( head->tail_node ).next = tmp; //将原队尾的后一个元素设置为数组free_head_node
            head_->tail_node = tmp;
        } else {									//队列为空
            head_->head_node = tmp;
            head_->tail_node = tmp;
        }

        size ++;
        node.data = elem;
        node.next = -1;
        return true;
    }
}

MY_TEMPLATE
void QUEUE::Pop()
{
    if( size ) {
        ShmQueueNode node;
        node = array_.Get( head_->head_node );
        ns_shm_data::T_SHM_SIZE tmp = head_->head_node;
        head_->head_node = node.next;
        node->next = head_->free_head_node;
        head_->free_head_node = tmp;

        size --;
    }
}

MY_TEMPLATE
void QUEUE::Clear()
{
    new( head_ ) ShmQueueHead;
    array_.Clear();
    return true;
}

MY_TEMPLATE
int QUEUE::Size()
{
    return head_->size;
}

MY_TEMPLATE
bool QUEUE::IsEmpty()
{
    return head_->size <= 0;
}

MY_TEMPLATE
bool QUEUE::SetCreateShm( long n )
{
    if( !m_array._CreateShm() ) {
        return false;
    }

    if( !SetArrayToShm() ) {
        return false;
    }

    new( array_.GetUserHead() ) ShmQueueHead;
    clear();

    if( !SetDetachFromShm() ) {
        return false;
    }

    return true;
}

MY_TEMPLATE
bool QUEUE::SetAttachToShm()
{
    if( !array_.AttachToShm( false ) ) {
        return false;
    }

    head_ = array_.GetUserHead();
    return true;
}

MY_TEMPLATE
bool QUEUE::SetDetachFromShm()
{
    if( array_.DetachFromShm() ) {
        return false;
    }

    head_ = NULL;
    return true;
}
