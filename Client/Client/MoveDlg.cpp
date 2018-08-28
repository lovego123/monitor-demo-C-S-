#include "stdafx.h"
#include "MoveDlg.h"


MoveDlg::MoveDlg()
{
}


MoveDlg::~MoveDlg()
{
}
void MoveDlg::OnSize(const CDialog * dlg, CRect &rect, UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED) //判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
	{
		CWnd* pChildWnd = dlg->GetWindow(GW_CHILD);
		UINT nCtrlID = 0;
		while (pChildWnd != NULL) //遍历对每个控件做改变
		{
			nCtrlID = pChildWnd->GetDlgCtrlID();
			ChangeSize(dlg, rect, nCtrlID, cx, cy);
			pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
		}

		dlg->GetClientRect(&rect); //最后要更新对话框的大小，当做下一次变化的旧坐标
	}
}

void MoveDlg::ChangeSize(const CDialog *dlg, CRect &m_rect, UINT nID, int x, int y)//nID为控件ID，x, y分别为对话框的当前长和宽
{
	CWnd *pWnd = dlg->GetDlgItem(nID);
	if (pWnd != NULL) //判断是否为空，因为在窗口创建的时候也会调用OnSize函数，但是此时各个控件还没有创建，Pwnd为空
	{
		CRect rec;
		pWnd->GetWindowRect(&rec); //获取控件变化前的大小
		dlg->ScreenToClient(&rec); //将控件大小装换位在对话框中的区域坐标
		rec.left = rec.left*x / m_rect.Width(); //按照比例调整空间的新位置
		rec.top = rec.top*y / m_rect.Height();
		rec.bottom = rec.bottom*y / m_rect.Height();
		rec.right = rec.right*x / m_rect.Width();
		pWnd->MoveWindow(rec); //伸缩控件
	}
}