const aboutBtn = document.getElementById('aboutBtn');
const aboutModal = document.getElementById('aboutModal');
const closeModal = document.getElementById('closeModal');

aboutBtn.addEventListener('click', () => {
   aboutModal.style.display = 'flex';
});

closeModal.addEventListener('click', () => {
   aboutModal.style.display = 'none';
});

// close modal when clicking outside the modal content
aboutModal.addEventListener('click', (e) => {
   if (e.target === aboutModal) {
      aboutModal.style.display = 'none';
   }
});

const data = [
  { emoji: "✊", word: "ROCK" },
  { emoji: "✋", word: "PAPER" },
  { emoji: "✌️", word: "SCISSORS" }
];

const scr   = document.getElementById("scr");
const glyph = document.getElementById("glyph");
let timer;
const typeSound = new Audio("https://www.soundjay.com/communication/computer-keyboard-1.wav");

scr.textContent = "";

function play () {
  clearTimeout(timer);
  typeSound.pause();
  typeSound.currentTime = 0;

  const item = data[Math.random() * data.length | 0];
  let idx = 0;

  typeSound.loop = true;
  typeSound.play().catch(()=>{});

  (function flash () {
    if (idx < item.word.length) {
      scr.textContent = item.word[idx];
      idx++;
      timer = setTimeout(flash, 250);
    } else {
      typeSound.pause();
      typeSound.currentTime = 0;
      scr.textContent = item.emoji;
    }
  })();
}

glyph.addEventListener("click", play);


document.addEventListener('DOMContentLoaded', () => {

   // SPA navigation
   const links = document.querySelectorAll('nav a[href^="#"]');
   const sections = document.querySelectorAll('section');

   const showSection = (id) => {
      sections.forEach(s => s.classList.remove('active'));
      const target = document.getElementById(id);
      if (target) target.classList.add('active');
   };

   links.forEach(link => {
      link.addEventListener('click', (e) => {
         e.preventDefault();
         const href = link.getAttribute('href');
         if (href.startsWith('#')) {
            showSection(href.slice(1));
         }
      });
   });

   // Background toggle
   const bgToggle = document.getElementById('bgToggle');
   bgToggle.addEventListener('click', () => {
      document.body.classList.toggle('hideBg');
      bgToggle.textContent = document.body.classList.contains('hideBg') ? 'show bg' : 'hide bg';
   });

   // Neural network mouse repel
   const circles = document.querySelectorAll('.mesh circle[data-repel]');
   const lines = document.querySelectorAll('.mesh line');

   document.addEventListener('mousemove', (e) => {
      const rect = document.querySelector('.mesh').getBoundingClientRect();
      const mouseX = e.clientX - rect.left;
      const mouseY = e.clientY - rect.top;

      circles.forEach(circle => {
         const cx = parseFloat(circle.getAttribute('cx'));
         const cy = parseFloat(circle.getAttribute('cy'));
         const distance = Math.sqrt(Math.pow(mouseX - cx, 2) + Math.pow(mouseY - cy, 2));

         if (distance < 100) {
            const repelForce = (100 - distance) / 100;
            const angle = Math.atan2(cy - mouseY, cx - mouseX);
            const repelX = Math.cos(angle) * repelForce * 20;
            const repelY = Math.sin(angle) * repelForce * 20;

            circle.style.transform = `translate(${repelX}px, ${repelY}px)`;

            // Affect connected lines
            lines.forEach(line => {
               const x1 = parseFloat(line.getAttribute('x1'));
               const y1 = parseFloat(line.getAttribute('y1'));
               const x2 = parseFloat(line.getAttribute('x2'));
               const y2 = parseFloat(line.getAttribute('y2'));

               if ((x1 === cx && y1 === cy) || (x2 === cx && y2 === cy)) {
                  line.style.strokeOpacity = Math.max(0.05, 0.12 - repelForce * 0.1);
               }
            });
         } else {
            circle.style.transform = '';
            lines.forEach(line => {
               line.style.strokeOpacity = '';
            });
         }
      });
   });

   // Uiverse switch
   const switchInput = document.getElementById('switchInput');
   const switchContainer = document.getElementById('switchContainer');
   const switchLabel = document.getElementById('switchLabel');

   switchInput.addEventListener('change', () => {
      const isChecked = switchInput.checked;
      switchContainer.classList.toggle('checked', isChecked);
      switchLabel.classList.toggle('checked', isChecked);
      document.body.classList.toggle('alt', isChecked);

      // Play sound
      const sound = document.getElementById(isChecked ? 'soundOn' : 'soundOff');
      sound.play().catch(e => console.log('Sound play failed:', e));
   });

   // Spinner
   const spinner = document.getElementById('spinner');

   spinner.addEventListener('click', () => {
      const duration = 3000; // 3 seconds spin
      const easingFrames = 60;
      let angle = 0;
      let frame = 0;

      const animateSpin = () => {
         const t = frame / easingFrames;
         const eased = Math.pow(1 - t, 3); // cubic ease-out
         angle += eased * 20;
         spinner.style.transform = `rotate(${angle}deg)`;

         frame++;
         if (frame <= easingFrames) {
            requestAnimationFrame(animateSpin);
         }
      };

      animateSpin();
   });

   // Card door
   const door = document.getElementById('cardDoor');
   let cardOpen = false;

   door.addEventListener('click', () => {
      cardOpen = !cardOpen;
      door.classList.toggle('open', cardOpen);

      const sound = document.getElementById(cardOpen ? 'soundCardOpen' : 'soundCardClose');
      sound.currentTime = 0;
      sound.play().catch(e => console.log('Card sound failed:', e));
   });
});

const darkToggle = document.getElementById('darkToggle');
darkToggle.addEventListener('click', () => {
  document.body.classList.toggle('dark');
  darkToggle.textContent = document.body.classList.contains('dark') ? 'light mode' : 'dark mode';
});

// lor request modal
(function () {
   function decodeEmail(obfuscated) {
      return obfuscated
         .replace(/\[dot\]/g, '.')
         .replace(/\[at\]/g, '@');
   }
   const EMAIL_SAFE = 'aaditya[dot]rengarajan[at]nyu[dot]edu';

   const lorOverlay = document.getElementById('lorOverlay');
   const lorIssuerName = document.getElementById('lorIssuerName');
   const lorForm = document.getElementById('lorForm');
   const lorName = document.getElementById('lorName');
   const lorAffiliation = document.getElementById('lorAffiliation');
   const lorPurpose = document.getElementById('lorPurpose');
   const lorError = document.getElementById('lorError');
   const lorMailBtn = document.getElementById('lorMailBtn');
   const lorCancel = document.getElementById('lorCancel');

   let currentIssuer = { name: '', org: '' };

   function openLorModal(issuerName, issuerOrg) {
      currentIssuer = { name: issuerName, org: issuerOrg };
      lorIssuerName.textContent = issuerName + ' (' + issuerOrg + ')';
      lorForm.reset();
      lorForm.style.display = '';
      lorMailBtn.classList.remove('show');
      lorError.classList.remove('show');
      lorOverlay.classList.add('open');
   }

   function closeLorModal() {
      lorOverlay.classList.remove('open');
   }

   function buildLorMailto() {
      const name = lorName.value.trim();
      const affiliation = lorAffiliation.value.trim();
      const purpose = lorPurpose.value.trim();

      const to = decodeEmail(EMAIL_SAFE);
      const subject = 'Request to view LoR';
      const body =
         'Hi Aaditya,\n\n' +
         'I was browsing through your web portfolio and would like to request your LoR from ' +
         currentIssuer.name + ' (' + currentIssuer.org + ').\n\n' +
         purpose + '\n\n' +
         'Thanks,\n' +
         name + '\n' +
         affiliation;

      return 'mailto:' + to +
         '?subject=' + encodeURIComponent(subject) +
         '&body=' + encodeURIComponent(body);
   }

   lorForm.addEventListener('submit', (e) => {
      e.preventDefault();
      if (!lorName.value.trim() || !lorAffiliation.value.trim() || !lorPurpose.value.trim()) {
         lorError.classList.add('show');
         return;
      }
      lorError.classList.remove('show');
      lorMailBtn.href = buildLorMailto();
      lorMailBtn.classList.add('show');
      lorForm.style.display = 'none';
   });

   lorCancel.addEventListener('click', closeLorModal);
   lorOverlay.addEventListener('click', (e) => {
      if (e.target === lorOverlay) closeLorModal();
   });

   document.querySelectorAll('.lor-item').forEach((item) => {
      const link = item.querySelector('a');
      if (!link) return;

      const rawText = item.textContent.split(link.textContent)[0].trim();
      const parts = rawText.split('–');
      const issuerName = (parts[0] || '').trim();
      const issuerOrg = (parts[1] || '').trim();

      link.addEventListener('click', (e) => {
         e.preventDefault();
         openLorModal(issuerName, issuerOrg);
      });
   });
})();

document.querySelectorAll('a[href^="mailto"]').forEach(link => {
  const href = link.getAttribute('href');
  const decoded = href.replace(/\[dot\]/g, '.').replace(/\[at\]/g, '@').replace(/\[colon\]/g, ':');
  
  link.addEventListener('click', (e) => {
    e.preventDefault();
    window.open(decoded);
  });
  
  link.setAttribute('href', '#');
});

(function(){
 var bar = document.getElementById('notifBar');
 if(!bar) return;
 if(localStorage.getItem('sshNotifClosed') === '1'){
   bar.classList.add('hidden');
 } else {
   document.getElementById('notifClose').addEventListener('click', function(){
     bar.classList.add('hidden');
     localStorage.setItem('sshNotifClosed', '1');
   });
 }

 document.getElementById('sshTrigger').addEventListener('click', function(){
   var ok = confirm('copy "ssh aadi.zip" to clipboard and try launching your terminal?');
   if(!ok) return;

   copyCommand();

   var iframe = document.createElement('iframe');
   iframe.style.display = 'none';
   iframe.src = 'ssh://aadi.zip';
   document.body.appendChild(iframe);
   setTimeout(function(){ iframe.remove(); }, 1000);
 });

 function copyCommand(){
   var text = 'ssh aadi.zip';
   if(navigator.clipboard){
     navigator.clipboard.writeText(text).then(showToast, function(){
       fallbackCopy(text);
     });
   } else {
     fallbackCopy(text);
   }
 }

 function fallbackCopy(text){
   var ta = document.createElement('textarea');
   ta.value = text;
   ta.style.position = 'fixed';
   ta.style.opacity = '0';
   document.body.appendChild(ta);
   ta.select();
   try { document.execCommand('copy'); } catch(e) {}
   document.body.removeChild(ta);
   showToast();
 }

 function showToast(){
   var t = document.getElementById('sshToast');
   t.classList.add('show');
   setTimeout(function(){ t.classList.remove('show'); }, 3000);
 }
})();