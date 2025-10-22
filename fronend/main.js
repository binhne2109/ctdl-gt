async function api(path, opts={}){
  const res = await fetch(path, opts);
  if(!res.ok) throw new Error(res.status);
  if(res.headers.get('Content-Type') && res.headers.get('Content-Type').includes('application/json')) return res.json();
  return res.text();
}

let cards = [];
let idx = 0;

async function load(){
  cards = await api('/cards');
  if(!Array.isArray(cards)) cards = [];
  idx = Math.max(0, Math.min(idx, cards.length-1));
  render();
}

function setCardFlip(state){
  const el = document.getElementById('card');
  if(state) el.classList.add('show-back'); else el.classList.remove('show-back');
}

function render(){
  const front = document.getElementById('front');
  const back = document.getElementById('back');
  const list = document.getElementById('list');
  if(cards.length===0){ front.textContent='(chưa có thẻ)'; back.textContent=''; }
  else{
    front.textContent = cards[idx].front;
    back.textContent = cards[idx].back;
  }
  setCardFlip(false);
  list.innerHTML = '';
  cards.forEach((c,i)=>{
    const li = document.createElement('li');
    li.textContent = (i+1)+'. '+c.front;
    li.addEventListener('click', ()=>{ idx = i; setCardFlip(false); render(); });
    if(i===idx) li.classList.add('current');
    list.appendChild(li);
  });
}

async function addCard(){
  const f = document.getElementById('newFront').value.trim();
  const b = document.getElementById('newBack').value.trim();
  if(!f) return alert('Nhập mặt trước');
  await api('/cards', { method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({front:f, back:b}) });
  document.getElementById('newFront').value=''; document.getElementById('newBack').value='';
  await load();
}

async function delCard(){
  if(cards.length===0) return;
  const id = cards[idx].id;
  await api('/cards?id='+id, { method:'DELETE' });
  idx = Math.max(0, idx-1);
  await load();
}

function next(){ if(cards.length===0) return; idx = Math.min(idx+1, cards.length-1); setCardFlip(false); render(); }
function prev(){ if(cards.length===0) return; idx = Math.max(idx-1,0); setCardFlip(false); render(); }
function flip(){ const el = document.getElementById('card'); const show = el.classList.toggle('show-back'); }

window.addEventListener('load', ()=>{
  document.getElementById('next').addEventListener('click', ()=>next());
  document.getElementById('prev').addEventListener('click', ()=>prev());
  document.getElementById('flip').addEventListener('click', ()=>flip());
  document.getElementById('addBtn').addEventListener('click', ()=>addCard());
  document.getElementById('del').addEventListener('click', ()=>delCard());
  document.getElementById('card').addEventListener('click', ()=>flip());
  load().catch(e=>console.error(e));
});