/**
Copyright (c) 2009-2012 GUANGDONG ESHORE TECHNOLOGY CO., LTD.
All rights reserved.

Created:    2012.9.1 16:41
Module:      shm_queue.h
Author:      瞿兆静
Revision:    v1.0
Description: 在Array的基础上封装一个Queue
*/

#ifndef SHM_QUEUE_H_

#define SHM_QUEUE_H_

#include "shm_Array.h"

namespace tpss
{

//队列结点
template <typename T_DATA>
struct ShmQueueNode {
    ShmQueueNode( T_DATA data = T_DATA(), ns_shm_data::T_SHM_SIZE next = -1 ): data( data ), next( -1 ) {
    }
    ns_shm_data::T_SHM_SIZE next;	//指向排在其后面的数据的所在数组位置
    T_DATA data;		//队列结点携带的数据元
};

//队列头部
struct ShmQueueHead {
    ShmQueueHead(): size( 0 ), head_node( 0 ), tail_node( 0 ), free_head_node( -1 ) {

    }
    //记录队列的尺寸
    int size;

    //记录头和尾的位置
    ns_shm_data::T_SHM_SIZE head_node;
    ns_shm_data::T_SHM_SIZE tail_node;

    //回收栈，维持出队列的数据空间回收，其也是一个队列， 也有头和尾
    ns_shm_data::T_SHM_SIZE free_head_node;
};

//队列
template<typename T_DATA, int PI_N, typename T_USER_HEAD = ns_shm_data::CDemoData, int VER = 0, typename T_HANDLE = ns_shm_data::T_HANDLE_ARRAY<ShmQueueNode<T_DATA> , PI_N> >
class ShmQueue
{
public:
    ShmQueue( char const* name, int version ): array_( name, version ) {
    }
    ~ShmQueue() {
    }

    //获取共享内存的名称
    char const* GetName()const {
        array_.GetName();
    }

    //创建共享内存
    bool CreateShm() {
        SetCreateShm( 0 );
    }

    //加载共享内存的数据
    bool Attach( bool is_read_only ) {
        return SetAttachToShm();
    }
    //释放共享内存里面的数据
    bool Detach() {
        return SetDetachFromShm();
    }
    //获取队头的数据元素。返回值为空，表示队列为空; 返回值非空，获取队列头部元素成功。
    T_DATA* Top() {
        if( IsEmpty() ) {
            return 0;
        }

        return &array_.Get( head_->head_node )->data;
    }
    //向队尾压入一个数据。返回值为false，表示压入数据失败；返回值为true，压入数据成功
    bool Push( const T_DATA& elem ) {
        bool ret = true;

        if( -1 == head_->free_head_node ) {
            //回收队列为空，说明[0, size)单元数组全部装有有效数据， 只需把当前元素插入到size位置就行
            T_HANDLE h( head_->size );
            ShmQueueNode<T_DATA> node( elem );
            ret = array_.Add( node, h );

            if( ret ) {
                if( head_->size ) {
                    array_.Get( head_->tail_node )->next = head_->size;
                    head_->tail_node = head_->size;
                } else {
                    head_->tail_node = head_->size;
                    head_->head_node = head_->size;
                }
            }

            head_->size ++;
            return ret;
        } else {
            ShmQueueNode<T_DATA>* ptr_node;
            //回收栈不为空，将现有数据加入到回收队列的队首位置
            ptr_node = array_.Get( head_->free_head_node );	//获取回收栈的数组单元
            ns_shm_data::T_SHM_SIZE tmp = head_->free_head_node;
            head_->free_head_node = ptr_node->next;			//回收栈弹出一个数组单元

            if( head_->size ) {									//队列为非空
                array_.Get( head_->tail_node )->next = tmp; //将原队尾的后一个元素设置为数组free_head_node
                head_->tail_node = tmp;
            } else {									//队列为空
                head_->head_node = tmp;
                head_->tail_node = tmp;
            }

            head_->size ++;
            ptr_node->data = elem;
            ptr_node->next = -1;
            return true;
        }

        return ret;
    }
    //向队头弹出一个数据
    void Pop() {
        if( head_->size ) {
            ShmQueueNode<T_DATA>* ptr_node;
            ptr_node = array_.Get( head_->head_node );
            ns_shm_data::T_SHM_SIZE tmp = head_->head_node;
            head_->head_node = ptr_node->next;
            ptr_node->next = head_->free_head_node;
            head_->free_head_node = tmp;

            -- head_->size;
        }
    }
    //将队列中的数据清空
    void Clear() {
        new( head_ ) ShmQueueHead;
        array_.Clear();
        head_->size = 0;
        head_->head_node = 0;
        head_->tail_node = 0;
        head_->free_head_node = -1;
    }
    //获取队列的尺寸
    int Size() {
        return head_->size;
    }
    //判断队列是否为空
    bool IsEmpty() {
        return head_->size <= 0;
    }
private:
    bool SetCreateShm( long n ) {
        if( !array_.CreateShm() ) {
            printf( "CreateShm失败\n" );
            return false;
        }

        if( !SetAttachToShm() ) {
            printf( "SetAttachToShm失败\n" );
            return false;
        }

        new( array_.GetUserHead() ) ShmQueueHead;
        Clear();

        if( !SetDetachFromShm() ) {
            printf( "SetDetachFromShm失败\n" );
            return false;
        }

        return true;
    }

    bool SetAttachToShm() {
        if( !array_.AttachToShm( false ) ) {
            return false;
        }

        head_ = array_.GetUserHead();
        return true;
    }

    bool SetDetachFromShm() {
        if( !array_.DetachFromShm() ) {
            return false;
        }

        head_ = NULL;
        return true;
    }

    ShmQueueHead* head_;//队列头部信息，记录了队列的一些属性
    ns_shm_data::T_ARRAY<ShmQueueNode<T_DATA>, PI_N, ShmQueueHead, T_HANDLE> array_;//这个队列是基于数组的
};
};

#endif
