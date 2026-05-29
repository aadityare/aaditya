package main

import (
	"context"
	"fmt"
	"math/rand"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	tea "github.com/charmbracelet/bubbletea"
	"github.com/charmbracelet/lipgloss"
	"github.com/charmbracelet/ssh"
	"github.com/charmbracelet/wish"
	"github.com/charmbracelet/wish/bubbletea"
	"github.com/charmbracelet/wish/logging"
)

const (
	listenHost  = "127.0.0.1"
	listenPort  = 2222
	keyPath     = "server/ssh-server/host_key"
	starRows    = 2
	matrixRows  = 2
)

const (
	colorOrange     = "173"
	colorBoba       = "94"
	colorCream      = "255"
	colorDim        = "138"
	colorMatrixDim  = "137"
	colorMatrixBrt  = "145"
	colorWhite      = "15"
)

type paper struct{ title, abstract, doi string }

var papers = []paper{
	{
		title: "Estimation of Warfarin Dosage using a Specialized XGBoost-based Pharmacogenomic Machine Learning Model and Evaluation using XAI",
		abstract: `Warfarin dosing remains clinically challenging due to its narrow therapeutic
window and high interpatient variability driven by genetic and clinical factors.
This paper presents a specialized XGBoost-based pharmacogenomic model integrating
CYP2C9 and VKORC1 polymorphisms with clinical variables to predict stable warfarin
doses. Explainability is evaluated using XAI techniques to surface the most
influential features, offering clinicians interpretable, personalized dosing
recommendations.`,
		doi: "https://doi.org/10.1109/icmlc66258.2025.11280120",
	},
	{
		title: "CYBRANA – YARA/YAML-driven AI Firewall",
		abstract: `CYBRANA is an AI-powered firewall framework leveraging YARA rules and YAML
configuration for dynamic cyber attack detection and log analysis. The system
integrates MITRE ATT&CK framework mappings for automated threat classification.
Evaluation across real-world network traffic demonstrates 91.7% attack classification
accuracy with sub-millisecond detection latency — suitable for enterprise-grade
security operations at scale.`,
		doi: "https://doi.org/10.1109/CVMI61877.2024.10782249",
	},
	{
		title: "FLARE: Federated Learning And Resilient Encryption for Firewalls",
		abstract: `Traditional firewalls rely on static rule-based mechanisms that are inherently
limited and easily bypassed by evolving malware. FLARE proposes a federated
learning-based firewall solution that dynamically analyzes past network traffic
patterns across distributed nodes, combined with resilient encryption to preserve
data privacy. The framework improves adaptability to novel threats while maintaining
strong confidentiality guarantees across heterogeneous network environments.`,
		doi: "https://doi.org/10.1109/punecon63413.2024.10895282",
	},
	{
		title: "SHADOW: A framework for Systematic Heuristic Analysis and Detection of Observations on the Web",
		abstract: `Cyberspace contains vast amounts of information critical for threat intelligence,
attack prevention, and organizational security. SHADOW proposes a data ingestion
and storage pipeline integrating surface web, deep web, and dark web sources via
advanced scraping techniques. Collected data is transformed and stored in a private
cloud and Proof-of-Authority blockchain for immutability. An NLP pipeline analyzes
and ranks results using a custom algorithm to surface actionable threat intelligence
and detect organizational data exposure in real time.`,
		doi: "https://doi.org/10.1109/icamac62387.2024.10828750",
	},
	{
		title: "Enhancing the Resilience of Privacy-Preserving Machine Learning using Adversarial Techniques",
		abstract: `Privacy-preserving machine learning systems face compounding threats from
adversarial attacks and privacy inference vulnerabilities. This paper proposes a
framework integrating adversarial techniques with differential privacy and
homomorphic encryption to enhance resilience. The approach hardens ML models
against evasion and membership inference attacks while preserving data
confidentiality, evaluated across multiple threat models in distributed learning
settings.`,
		doi: "https://doi.org/10.1109/icdscnc62492.2024.10939481",
	},
}

type alink struct{ label, sub, url string }

var aboutLinks = []alink{
	{"resume",    "industry recruiters",  "https://aadi.zip/resume"},
	{"cv",        "academic evaluators",  "https://aadi.zip/cv"},
	{"portfolio", "freelance work",       "https://aadi.zip/portfolio"},
	{"linkedin",  "professional network", "https://linkedin.com/in/aaditya-rengarajan"},
	{"email",     "direct contact",       "mailto:aaditya.r@ieee.org"},
}

type rpsState int

const (
	rpsIdle rpsState = iota
	rpsCountdown
	rpsRevealing
	rpsDone
)

var rpsChoices = []string{"rock", "paper", "scissors"}

type rpsCountdownMsg int
type rpsRevealMsg    int

type page int

const (
	pageHome page = iota
	pageResearch
	pageResearchDetail
	pageAbout
	pageRPS
)

var navItems = []struct{ label string; pg page }{
	{"research",           pageResearch},
	{"about me",           pageAbout},
	{"rock paper scissors", pageRPS},
}

type star struct{ x, y int; ch string }

var starGlyphs = []string{"·", "✦", "✧", "+", "⋆", "∗"}

func makeStars(w int) []star {
	n := w / 8
	stars := make([]star, n)
	for i := range stars {
		stars[i] = star{
			x:  rand.Intn(w),
			y:  rand.Intn(starRows),
			ch: starGlyphs[rand.Intn(len(starGlyphs))],
		}
	}
	return stars
}

type matrixCol struct {
	chars []rune
	speed int
	tick  int
}

func makeMatrix(w int) []matrixCol {
	cols := make([]matrixCol, w)
	for i := range cols {
		chars := make([]rune, matrixRows)
		for j := range chars {
			chars[j] = []rune("01")[rand.Intn(2)]
		}
		cols[i] = matrixCol{chars: chars, speed: 1 + rand.Intn(4)}
	}
	return cols
}

type tickMsg struct{}

type model struct {
	rdr         *lipgloss.Renderer
	width       int
	height      int
	pg          page
	navSel      int
	resSel      int
	aboutSel    int
	stars       []star
	matrix      []matrixCol
	frame       int
	rps         rpsState
	rpsCD       int
	rpsChoice   string
	rpsRevealed int
}

func newModel(r *lipgloss.Renderer, w, h int) model {
	return model{
		rdr:    r,
		width:  w,
		height: h,
		stars:  makeStars(w),
		matrix: makeMatrix(w),
	}
}

func doTick() tea.Cmd {
	return tea.Tick(100*time.Millisecond, func(t time.Time) tea.Msg { return tickMsg{} })
}
func cdTick(n int) tea.Cmd {
	return tea.Tick(1*time.Second, func(t time.Time) tea.Msg { return rpsCountdownMsg(n) })
}
func revTick(i int) tea.Cmd {
	return tea.Tick(110*time.Millisecond, func(t time.Time) tea.Msg { return rpsRevealMsg(i) })
}

func (m model) Init() tea.Cmd { return doTick() }

func (m model) back() (model, tea.Cmd) {
	switch m.pg {
	case pageResearchDetail:
		m.pg = pageResearch
	default:
		m.pg = pageHome
		m.rps = rpsIdle
		m.rpsChoice = ""
		m.rpsRevealed = 0
		m.rpsCD = 0
	}
	return m, nil
}

func (m model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	switch msg := msg.(type) {

	case tea.WindowSizeMsg:
		m.width = msg.Width
		m.height = msg.Height
		m.stars = makeStars(m.width)
		m.matrix = makeMatrix(m.width)

	case tickMsg:
		m.frame++
		for i := range m.stars {
			if rand.Float32() < 0.08 {
				m.stars[i].ch = starGlyphs[rand.Intn(len(starGlyphs))]
			}
		}
		// matrix scroll
		for i := range m.matrix {
			m.matrix[i].tick++
			if m.matrix[i].tick >= m.matrix[i].speed {
				m.matrix[i].tick = 0
				chars := m.matrix[i].chars
				copy(chars, chars[1:])
				chars[len(chars)-1] = []rune("01")[rand.Intn(2)]
			}
		}
		return m, doTick()

	case rpsCountdownMsg:
		n := int(msg)
		m.rpsCD = n
		if n > 1 {
			return m, cdTick(n - 1)
		}
		m.rps = rpsRevealing
		m.rpsChoice = rpsChoices[rand.Intn(len(rpsChoices))]
		m.rpsRevealed = 0
		return m, revTick(0)

	case rpsRevealMsg:
		m.rpsRevealed = int(msg) + 1
		if m.rpsRevealed < len([]rune(m.rpsChoice)) {
			return m, revTick(m.rpsRevealed)
		}
		m.rps = rpsDone
		return m, nil

	case tea.KeyMsg:
		if msg.String() == "q" {
			return m, tea.Quit
		}
		if msg.String() == "esc" || msg.String() == "backspace" {
			return m.back()
		}
		switch msg.String() {
		case "up":
			switch m.pg {
			case pageHome:
				if m.navSel > 0 { m.navSel-- }
			case pageResearch:
				if m.resSel > 0 { m.resSel-- }
			case pageAbout:
				if m.aboutSel > 0 { m.aboutSel-- }
			}
		case "down":
			switch m.pg {
			case pageHome:
				if m.navSel < len(navItems)-1 { m.navSel++ }
			case pageResearch:
				if m.resSel < len(papers)-1 { m.resSel++ }
			case pageAbout:
				if m.aboutSel < len(aboutLinks)-1 { m.aboutSel++ }
			}
		case "enter":
			switch m.pg {
			case pageHome:
				m.pg = navItems[m.navSel].pg
			case pageResearch:
				m.pg = pageResearchDetail
			case pageRPS:
				if m.rps == rpsIdle || m.rps == rpsDone {
					m.rps = rpsCountdown
					m.rpsCD = 3
					m.rpsChoice = ""
					m.rpsRevealed = 0
					return m, cdTick(3)
				}
			}
		}
	}
	return m, nil
}

func (m model) c(color string) lipgloss.Style {
	return m.rdr.NewStyle().Foreground(lipgloss.Color(color))
}
func (m model) cb(color string) lipgloss.Style {
	return m.rdr.NewStyle().Foreground(lipgloss.Color(color)).Bold(true)
}

func (m model) renderStars() string {
	grid := make([][]string, starRows)
	for i := range grid {
		row := make([]string, m.width)
		for j := range row { row[j] = " " }
		grid[i] = row
	}
	for _, s := range m.stars {
		if s.x < m.width && s.y < starRows {
			grid[s.y][s.x] = m.c(colorDim).Render(s.ch)
		}
	}
	var sb strings.Builder
	for _, row := range grid {
		sb.WriteString(strings.Join(row, "") + "\n")
	}
	return sb.String()
}

func (m model) renderMatrix() string {
	var sb strings.Builder
	for row := 0; row < matrixRows; row++ {
		for col := 0; col < m.width && col < len(m.matrix); col++ {
			ch := string(m.matrix[col].chars[row])
			if row == matrixRows-1 {
				sb.WriteString(m.c(colorMatrixBrt).Render(ch))
			} else {
				sb.WriteString(m.c(colorMatrixDim).Render(ch))
			}
		}
		sb.WriteString("\n")
	}
	return sb.String()
}

func (m model) View() string {
	switch m.pg {
	case pageResearch:       return m.viewResearch()
	case pageResearchDetail: return m.viewResearchDetail()
	case pageAbout:          return m.viewAbout()
	case pageRPS:            return m.viewRPS()
	default:                 return m.viewHome()
	}
}

func pad(n int) string { return strings.Repeat(" ", n) }

func (m model) viewHome() string {
	var b strings.Builder
	indent := "    "

	b.WriteString(m.renderStars())
	b.WriteString("\n")

	// name
	b.WriteString(indent + m.cb(colorOrange).Render("aaditya rengarajan") + "\n")
	b.WriteString(indent + m.c(colorDim).Render("cybersecurity & ai researcher") + "\n")
	b.WriteString("\n")

	// bio
	b.WriteString(indent + m.c(colorCream).Render("ms cybersecurity, new york university.") + "\n")
	b.WriteString(indent + m.c(colorCream).Render("i work at the crossroads of offensive security,") + "\n")
	b.WriteString(indent + m.c(colorCream).Render("agentic ai systems, and large-scale automation.") + "\n")
	b.WriteString("\n")
	b.WriteString(indent + m.c(colorDim).Render("formerly at intel corporation, isro, isac,") + "\n")
	b.WriteString(indent + m.c(colorDim).Render("and equate petrochemical company.") + "\n")
	b.WriteString("\n")
	b.WriteString(indent + m.c(colorCream).Render("i've contributed to gov-grade threat intelligence") + "\n")
	b.WriteString(indent + m.c(colorCream).Render("platforms, deep rl research for intel foundry ops,") + "\n")
	b.WriteString(indent + m.c(colorCream).Render("and led cybersec education for 500+ learners.") + "\n")
	b.WriteString("\n\n")

	// nav — only research + about me on home
	for i, item := range navItems {
		if i == m.navSel {
			b.WriteString(indent + m.cb(colorBoba).Render("›  "+item.label) + "\n")
		} else {
			b.WriteString(indent + m.c(colorDim).Render("   "+item.label) + "\n")
		}
	}

	b.WriteString("\n")
	b.WriteString(indent + m.c(colorDim).Render("↑↓ navigate   enter select   q quit") + "\n")
	b.WriteString("\n")
	b.WriteString(m.renderMatrix())
	return b.String()
}

func (m model) viewResearch() string {
	indent := "    "
	var b strings.Builder
	b.WriteString(m.renderStars())
	b.WriteString("\n")
	b.WriteString(indent + m.cb(colorOrange).Render("research") + "\n")
	b.WriteString(indent + m.c(colorDim).Render("published work") + "\n\n")

	for i, p := range papers {
		if i == m.resSel {
			b.WriteString(indent + m.cb(colorBoba).Render(fmt.Sprintf("›  [%d]  %s", i+1, p.title)) + "\n")
			b.WriteString(indent + m.c(colorDim).Render("       enter to read") + "\n\n")
		} else {
			b.WriteString(indent + m.c(colorDim).Render(fmt.Sprintf("   [%d]  ", i+1)) + m.c(colorCream).Render(p.title) + "\n\n")
		}
	}

	b.WriteString("\n" + indent + m.c(colorDim).Render("↑↓ navigate   enter open   esc back   q quit") + "\n\n")
	b.WriteString(m.renderMatrix())
	return b.String()
}

func (m model) viewResearchDetail() string {
	indent := "    "
	p := papers[m.resSel]
	var b strings.Builder
	b.WriteString(m.renderStars())
	b.WriteString("\n")
	b.WriteString(indent + m.cb(colorOrange).Render(p.title) + "\n\n")
	b.WriteString(indent + m.c(colorDim).Render("abstract") + "\n\n")
	for _, line := range strings.Split(p.abstract, "\n") {
		b.WriteString(indent + m.c(colorCream).Render(line) + "\n")
	}
	b.WriteString("\n")
	b.WriteString(indent + m.c(colorDim).Render("doi  ") + m.cb(colorBoba).Render(p.doi) + "\n")
	b.WriteString("\n" + indent + m.c(colorDim).Render("esc back   q quit") + "\n\n")
	b.WriteString(m.renderMatrix())
	return b.String()
}

func (m model) viewAbout() string {
	indent := "    "
	var b strings.Builder
	b.WriteString(m.renderStars())
	b.WriteString("\n")
	b.WriteString(indent + m.cb(colorOrange).Render("about me") + "\n")
	b.WriteString(indent + m.c(colorDim).Render("aaditya rengarajan") + "\n\n")

	for i, lnk := range aboutLinks {
		if i == m.aboutSel {
			b.WriteString(indent + m.cb(colorBoba).Render("›  "+lnk.label) + "\n")
			b.WriteString(indent + m.c(colorDim).Render("   "+lnk.sub) + "\n")
			b.WriteString(indent + m.c(colorCream).Render("   "+lnk.url) + "\n\n")
		} else {
			b.WriteString(indent + m.c(colorDim).Render("   "+lnk.label) + "\n")
			b.WriteString(indent + m.c(colorDim).Render("   "+lnk.sub) + "\n\n")
		}
	}

	b.WriteString("\n" + indent + m.c(colorDim).Render("↑↓ navigate   esc back   q quit") + "\n\n")
	b.WriteString(m.renderMatrix())
	return b.String()
}

func (m model) viewRPS() string {
	indent := "    "
	var b strings.Builder
	b.WriteString(m.renderStars())
	b.WriteString("\n")
	b.WriteString(indent + m.cb(colorOrange).Render("rock paper scissors") + "\n")
	b.WriteString(indent + m.c(colorDim).Render("the machine decides") + "\n\n")

	switch m.rps {
	case rpsIdle:
		b.WriteString(indent + m.c(colorCream).Render("press enter.  the machine will choose.") + "\n")

	case rpsCountdown:
		b.WriteString(indent + m.c(colorDim).Render("get ready") + "\n\n")
		b.WriteString(indent + m.cb(colorOrange).Render(fmt.Sprintf("%d", m.rpsCD)) + "\n")

	case rpsRevealing:
		runes := []rune(m.rpsChoice)
		revealed := string(runes[:m.rpsRevealed])
		b.WriteString(indent + m.c(colorDim).Render("the machine chose") + "\n\n")
		b.WriteString(indent + m.cb(colorCream).Render(revealed) + m.cb(colorBoba).Render("█") + "\n")

	case rpsDone:
		b.WriteString(indent + m.c(colorDim).Render("the machine chose") + "\n\n")
		b.WriteString(indent + m.cb(colorOrange).Render(strings.ToUpper(m.rpsChoice)) + "\n\n")
		b.WriteString(indent + m.c(colorDim).Render("enter to play again") + "\n")
	}

	b.WriteString("\n" + indent + m.c(colorDim).Render("esc back   q quit") + "\n\n")
	b.WriteString(m.renderMatrix())
	return b.String()
}

func handler(s ssh.Session) (tea.Model, []tea.ProgramOption) {
	pty, _, _ := s.Pty()
	w, h := pty.Window.Width, pty.Window.Height
	if w == 0 { w = 80 }
	if h == 0 { h = 24 }
	r := bubbletea.MakeRenderer(s)
	return newModel(r, w, h), []tea.ProgramOption{tea.WithAltScreen()}
}

func main() {
	addr := fmt.Sprintf("%s:%d", listenHost, listenPort)
	srv, err := wish.NewServer(
		wish.WithAddress(addr),
		wish.WithHostKeyPath(keyPath),
		wish.WithMiddleware(
			bubbletea.Middleware(handler),
			logging.Middleware(),
		),
	)
	if err != nil {
		fmt.Fprintf(os.Stderr, "[ssh-server] error: %v\n", err)
		os.Exit(1)
	}
	done := make(chan os.Signal, 1)
	signal.Notify(done, os.Interrupt, syscall.SIGINT, syscall.SIGTERM)
	fmt.Printf("[ssh-server] listening on %s\n", addr)
	go func() {
		if err := srv.ListenAndServe(); err != nil {
			fmt.Printf("[ssh-server] stopped: %v\n", err)
		}
	}()
	<-done
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	srv.Shutdown(ctx)
}